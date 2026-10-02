//UGLY JSON PARSER
// But there is more ugly parser than this


#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <variant>

#include <Windows.h>

namespace ug {
	enum {
		JO_MAX = 10000,
	};
	enum JsonDataType : uint32_t {
		JDT_null,
		JDT_bool,
		JDT_int,
		JDT_num,
		JDT_str,
		JDT_arr,
		JDT_obj,
	};
	enum JsonTokType : uint32_t {
		JT_None,
		JT_Block,
		JT_Block_e,
		JT_Sq,
		JT_Sq_e,
		JT_Minus, //-
		JT_Str,
		JT_Int,
		JT_Num,
		JT_Ident,
		JT_Comma,
		JT_Colon,
		JT_true,
		JT_false,
		JT_null,
	};

	void PrintUtf8(const std::string& utf8)
	{
		static std::wstring g_wstr;
		if (utf8.empty()) return;

		int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), nullptr, 0);
		g_wstr.resize(utf8.size());
		//std::wstring wstr(wlen, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), g_wstr.data(), wlen);

		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD written = 0;
		WriteConsoleW(hOut, g_wstr.data(), (DWORD)g_wstr.size(), &written, nullptr);
	}



	class MyJsonDataRaw;
	typedef MyJsonDataRaw MyJsonData;

	typedef std::unordered_map<std::string, MyJsonData>* MyJsonObj;
	typedef std::vector<MyJsonData>* MyJsonArr;
	typedef struct myjsondata {
		JsonDataType type : 31;
		JsonDataType moved : 1;
		union {
			long long i;
			double num;
			MyJsonObj obj;
			MyJsonArr arr;
			std::string* str;
			void* ptr;
		};
	} myjsondata;


	class MyJsonDataRaw : public myjsondata {
	private:
		void DeleteObj() {
			if (obj != nullptr) {
				for (auto& a : *obj) {
					a.second.Delete();
				}
				delete obj;
				obj = nullptr;
			}
		}
		template <typename T>
		void setVal(const T);
		void setVal(const long long ii) { i = ii; }
		void setVal(const bool b) { i = b; }
		void setVal(const double d) { num = d; }

	protected:
		void setRawData(myjsondata raw) {
			memcpy(this, &raw, sizeof(myjsondata));
		}
		void setRawData(MyJsonDataRaw rd) { *this = rd; }
		void setType(JsonDataType t) { type = t; moved = JDT_null; }
		void setNull() { setType(JDT_null); setVal(0ll); }
		void setBool(bool val) { setType(JDT_bool); setVal(val); }
		void setInt(long long val) { setType(JDT_int); setVal(val); }
		void setNum(double val) { setType(JDT_num); setVal(val); }
		bool setStr(const std::string& val) {
			setType(JDT_str);
			str = new std::string(val);
			return str != nullptr;
		}
		bool setArr() {
			setType(JDT_arr);
			arr = new std::vector<MyJsonData>();
			return arr != nullptr;
		}
		bool setArr(const std::vector<MyJsonData>& _arr) {
			setType(JDT_arr); arr = new std::vector<MyJsonData>(_arr);
			return arr != nullptr;
		}
		bool setObj() {
			setType(JDT_obj);
			obj = new std::unordered_map<std::string, MyJsonData>();
			return obj != nullptr;
		}
		bool setObj(const std::unordered_map<std::string, MyJsonData>& _obj) {
			type = JDT_obj;
			//void* ptr = MyJsonObject_new();
			obj = new std::unordered_map<std::string, MyJsonData>(_obj);
			return obj != nullptr;
		}



		void DeleteStr() {
			if (str != nullptr) {
				delete str;
				str = nullptr;
			}
		}
		void DeleteArr() {
			if (arr != nullptr) {
				for (auto& a : *arr) {
					a.Delete();
				}
				delete arr;
				arr = nullptr;
			}
		}
	public:
		MyJsonDataRaw GetRawData() const { return *this; }
		const JsonDataType GetType() const { return type; }
		long long GetInt() const { return i; }
		double GetNum() const { return num; }

		MyJsonArr GetArr() const {
			if (GetType() == JDT_arr) return arr;
			return nullptr;
		}
		MyJsonObj GetObj() const {
			if (GetType() == JDT_obj) return obj;
			return nullptr;
		}

		MyJsonArr GetArrPtr() const {
			return GetArr();
		}
		MyJsonObj GetObjPtr() const {
			return GetObj();
		}

		//void SetRawData() { Delete(); }
		void SetNull() { Delete(); setNull(); }
		void SetBool(bool val) {
			Delete(); setBool(val);
		}
		void SetInt(long long val) {
			Delete(); setInt(val);
		}
		void SetNum(double val) {
			Delete(); setNum(val);
		}
		bool SetStr(const std::string& _str) {
			Delete(); return setStr(_str);
		}
		bool SetArr() {
			Delete(); return setArr();
		}
		bool SetArr(const std::vector<MyJsonData>& _arr) {
			Delete(); return setArr(_arr);
		}
		bool SetObj() {
			Delete(); return setObj();
		}
		bool SetObj(const std::unordered_map<std::string, MyJsonData>& _obj) {
			Delete(); return setObj();
		}

		bool AddToObj(const std::string& key, const MyJsonData& dat) {
			if (GetType() == JDT_obj) {
				GetObj()->emplace(key, dat);
				return true;
			}
			return false;
		}
		bool AddToArr(const MyJsonData& dat) {
			if (GetType() == JDT_arr) {
				GetArr()->emplace_back(dat);
				return true;
			}
			return false;
		}

		void Init() {
			type = JDT_null;
			i = 0;
		}

		void Delete() {
			if ((int)moved == 1) return;
			switch (type) {
			case JDT_str: DeleteStr(); break;
			case JDT_arr: DeleteArr(); break;
			case JDT_obj: DeleteObj(); break;
			}
			type = JDT_null;
			i = 0;
		}

		void Dump() {
			switch (type) {
			case JDT_num:
				printf("%lf", num);
				break;
			case JDT_int:
				printf("%lld", i);
				break;
			case JDT_null:
				printf("null");
				break;
			case JDT_bool:
				printf("%s", (i ? "true" : "false"));
				break;
			case JDT_str:
				printf("\"%s\"", str->c_str());
				break;
			case JDT_arr:
				putchar('[');
				for (auto& a : *GetArr()) {
					a.Dump();
					putchar(',');
					putchar(' ');
				}
				putchar(']');
				break;
			case JDT_obj:
				putchar('{');
				for (auto& a : *GetObj()) {
					printf("%s : ", a.first.c_str());
					a.second.Dump();
					putchar(',');
					putchar(' ');
				}
				putchar('}');
				break;
			}
		}
		void DumpString(std::string& strbuf) {
			char buf[128];
			switch (type) {
			case JDT_num:
				sprintf_s(buf, 128, "%.16lf", num);
				for (int i = (int)strlen(buf) - 1; i >= 0; i--) {
					if (buf[i] == '0') continue;
					else if (buf[i] == '.') {
						//buf[i + 1] = '0';
						buf[i + 2] = '\0';
						break;
					}
					else {
						buf[i + 1] = '\0';
						break;
					}
				}
				strbuf += buf;
				break;
			case JDT_int:
				sprintf_s(buf, 128, "%lld", i);
				strbuf += buf;
				break;
			case JDT_null:
				strbuf += "null";
				break;
			case JDT_bool:
				if (i) {
					strbuf += "true";
				}
				else {
					strbuf += "false";
				}
				break;
			case JDT_str:
				strbuf += '"';
				strbuf += str->c_str();
				strbuf += '"';
				break;
			case JDT_arr:
				strbuf += '[';
				for (int i = 0; i < (int)arr->size() - 1; i++) {
					(*arr)[i].DumpString(strbuf);
					strbuf += ',';
				}
				if (arr->size() > 1) {
					(*arr)[arr->size() - 1].DumpString(strbuf);
				}
				strbuf += ']';
				break;
			case JDT_obj: {
				int cnt = 0;
				strbuf += '{';
				for (auto& a : *GetObj()) {
					strbuf += '"';
					strbuf += a.first.c_str();
					strbuf += '"';
					strbuf += ':';
					a.second.DumpString(strbuf);
					cnt++;
					if (cnt < obj->size()) strbuf += ',';
				}
				strbuf += '}';
				break;
			}
			}
		}

		void Move() {
			this->moved = JDT_bool;
		}
	};

	typedef std::unordered_map<std::string, MyJsonData> MyJsonObjData;
	typedef std::vector<MyJsonData> MyJsonArrData;

	template<class JSONTYPE>
	class MyJsonParser {
		std::string tokbuf;
		char* code;
		JSONTYPE tree;
		int code_len;

		int code_idx;

		JsonTokType toktype;
	public:
		static void InitMyJsonParser() {
			//if (g_JO == nullptr) {
			//	g_JO = new void* [JO_MAX];
			//}
		}	//
		static void DeleteMyJsonParser() {
			//if (g_JO != nullptr) {
			//	delete[] g_JO;
			//	g_JO = nullptr;
			//}
		}
		bool NewFromFile(const char* filename) {
			tokbuf.reserve(1024);
			code = nullptr;
			code_len = 0;
			code_idx = 0;

			FILE* fp;
			fopen_s(&fp, filename, "rb");
			if (fp == NULL) return false;
			long f_sz;
			fseek(fp, 0, SEEK_SET);
			fseek(fp, 0, SEEK_END);
			f_sz = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			code = new char[f_sz + 1];
			if (code == nullptr) return false;
			fread(code, sizeof(char), f_sz, fp);
			code[f_sz] = '\0';
			code_len = f_sz;
			//puts(code);
			fclose(fp);
			return true;
		}
		void Delete() {
			if (code != nullptr) {
				delete[] code;
				code = nullptr;
			}
			tree.Delete();
		}

		void AddUniToUtf8(uint16_t ch) {
			if (ch <= 0x007f) {
				tokbuf += (char)ch;
			}
			else if (ch <= 0x07ff) {
				tokbuf += (char)(ch & 0b011110000000);
				tokbuf += (char)(ch & 0b000001111111);
			}
			else if (ch <= 0x07ff) {
				tokbuf += (char)(((ch & 0b011110000000) >> 7) | 0b11000000);
				tokbuf += (char)((ch & 0b000001111111) | 0b10000000);
			}
			else {
				//yyyy|yxxxxx|xxxxxx
				tokbuf += (char)((ch >> 12) | 0b11100000);
				tokbuf += (char)(((ch >> 6) & 0b000111111) | 0b10000000);
				tokbuf += (char)((ch & 0b00111111) | 0b10000000);
			}
		}
		char Get() {
			return code[code_idx];
		}
		char Next() {
			if (code_idx == code_len)return '\0';
			code_idx++;
			return code[code_idx];
		}
		bool GetToken() {
			tokbuf.clear();
			char c = Get();
			while (isspace(c)) { c = Next(); }
			if (c == '\0') return false;
			if (isdigit(c)) {
				tokbuf += c;
				c = Next();
				while (isdigit(c)) {
					tokbuf += c;
					c = Next();
				}
				if (c != '.') {
					tokbuf += '\0';
					toktype = JT_Int;

				}
				else {
					tokbuf += c; c = Next();
					while (isdigit(c)) {
						tokbuf += c;
						c = Next();
					}
					tokbuf += '\0';
					toktype = JT_Num;
				}
			}
			else if (c == '"') {
				c = Next();
				while (c != '"' && c != '\0') {
					if (c == '\\') {
						c = Next();
						if (c == 'u') {
							uint16_t wc = 0u;
							c = Next();
							for (int i = 0; i < 4; i++) {
								wc *= 16u;
								if (isdigit(c)) { wc += c - '0'; }
								else if (c >= 'a' || c <= 'f') { wc += c - 'a' + 10; }
								else if (c >= 'A' || c <= 'F') { wc += c - 'A' + 10; }
								else return false;
								c = Next();
							}
							AddUniToUtf8(wc);
							continue;
						}
						else if (c == 'n') {
							c = '\n';
						}
						else if (c == 't') {
							c = '\t';
						}
						else if ((c == '\\') ||
								 (c == '"') ||
								 (c == '\'')) {

						}
						else {
							tokbuf += '\\';
						}
					}
					tokbuf += c;
					c = Next();
				}
				c = Next();
				tokbuf += '\0';
				toktype = JT_Str;
			}
			else if (c == '_' || isalpha(c)) {
				tokbuf += c;
				c = Next();
				while (c == '_' || isalpha(c) || isdigit(c)) {
					tokbuf += c;
					c = Next();
				}
				//c = Next();
				tokbuf += '\0';
				if (tokbuf == "true") toktype = JT_true;
				else if (tokbuf == "false") toktype = JT_false;
				else if (tokbuf == "null") toktype = JT_null;
				else toktype = JT_Ident;
			}
			else {
				if (c == '{') toktype = JT_Block;
				else if (c == '}') toktype = JT_Block_e;
				else if (c == '[') toktype = JT_Sq;
				else if (c == ']') toktype = JT_Sq_e;
				else if (c == ':') toktype = JT_Colon;
				else if (c == ',') toktype = JT_Comma;
				else if (c == '-') toktype = JT_Minus;
				else toktype = JT_None;
				tokbuf += c;
				tokbuf += '\0';
				Next();
			}
			//std::cout << code_idx << ':';
			//PrintUtf8(tokbuf);
			//std::cout << '\t';
			return true;
		}
		bool State(JSONTYPE* ptree) {
			JSONTYPE jd_buf;
			switch (toktype) {
			case JT_Block://{<str>:<factor>, ... }
				GetToken();
				ptree->SetObj();
				while (toktype != JT_Block_e && toktype != JT_None) {
					//GetToken();
					if (toktype != JT_Str && toktype != JT_Ident) return false;
					std::string key = tokbuf;
					GetToken();//:
					if (toktype != JT_Colon) return false;
					GetToken();
					ptree->AddToObj(key, std::move(jd_buf));
					if (!State(&(*ptree->GetObjPtr())[key])) return false;
					if (toktype != JT_Comma) break;
					GetToken();
				}
				if (toktype != JT_Block_e)return false;

				break;
			case JT_Sq:
				GetToken();
				ptree->SetArr();
				while (toktype != JT_Sq_e && toktype != JT_None) {
					ptree->AddToArr(std::move(jd_buf));
					if (!State(&ptree->GetArrPtr()->back())) return false;
					if (toktype != JT_Comma) break;
					GetToken();
				}
				if (toktype != JT_Sq_e)return false;
				break;
			case JT_Minus:
				GetToken();
				if (toktype == JT_Num) {
					ptree->SetNum(-atof(tokbuf.c_str()));
				}
				else if (toktype == JT_Int)
				{
					ptree->SetInt(-atoll(tokbuf.c_str()));
				}
				else return false;
				break;
			case JT_Str:case JT_Ident:
				ptree->SetStr(tokbuf);
				break;
			case JT_Int: {
				ptree->SetInt(atoll(tokbuf.c_str()));
				break;
			}
			case JT_Num: {
				ptree->SetNum(atof(tokbuf.c_str()));
				break;
			}
			case JT_null:
				ptree->SetNull();
				break;
			case JT_true:
				ptree->SetBool(true);
				break;
			case JT_false:
				ptree->SetBool(false);
				break;
			default:
				return false;
			}
			GetToken();
			return true;
		}
		bool Parse() {
			code_idx = 0;
			GetToken();
			if (!State(&tree)) return false;
			return true;
		}
		void Dump() {
			puts("json dump");
			tree.Dump();
		}
		void DumpString(std::string& str) {
			str.reserve(code_len);
			tree.DumpString(str);
		}
		MyJsonData GetJsonData() {
			return this->tree;
		}
	};
	//template <class T>
	//class MyJsonAlloc {
	//	using value_type = T;
	//	MyJsonAlloc() noexcept {}
	//	template <class U> MyJsonAlloc(const MyJsonAlloc<U>&) noexcept {}
	//
	//};

	class YourJsonData;
	typedef std::string YourJsonStr;
	typedef std::vector<YourJsonData>YourJsonArr;
	typedef std::unordered_map<std::string, YourJsonData> YourJsonObj;

	typedef std::unique_ptr<std::string> YourJsonStrPtr;
	typedef std::unique_ptr<YourJsonArr> YourJsonArrPtr;
	typedef std::unique_ptr<YourJsonObj> YourJsonObjPtr;

	struct YourJsonNull {
		bool b;
	};
	class YourJsonData {
	public:
		enum {
			YDT_Null,
			YDT_Bool,
			YDT_Int,
			YDT_Num,
			YDT_Str,
			YDT_Arr,
			YDT_Obj,
		};
		std::variant<
			YourJsonNull,
			bool,
			long long,
			double,
			YourJsonStrPtr,
			YourJsonArrPtr,
			YourJsonObjPtr> value;
		YourJsonData() { value = YourJsonNull(); }
		YourJsonData(const YourJsonNull) { value = YourJsonNull(); }
		template<typename T> YourJsonData(const T val);
		YourJsonData(const bool val) {
			value = val;
		}
		YourJsonData(const long long val) {
			value = val;
		}
		YourJsonData(const int val) {
			value = (long long)val;
		}
		YourJsonData(const double val) {
			value = val;
		}
		YourJsonData(const std::string& val) {
			value = std::make_unique<YourJsonStr>(val);
		}
		YourJsonData(const char* val) {
			value = std::make_unique<YourJsonStr>(val);
		}
		YourJsonData(YourJsonData&& val) {
			value = std::move(val.value);
		}

		~YourJsonData() {}

		void SetNull() { value = YourJsonNull(); }
		void SetNum(double d) { value = d; }
		void SetBool(bool b) { value = b; }
		void SetInt(long long i) { value = i; }
		void SetStr(const std::string& str) { 
			value = std::make_unique<YourJsonStr>(str);;
		}
		void SetArr() {
			value = std::make_unique<YourJsonArr>();
		}
		void SetObj() {
			value = std::make_unique<YourJsonObj>();
		}


		template<typename T> YourJsonData& operator=(const T);
		YourJsonData& operator=(const YourJsonNull) {
			value = YourJsonNull();
			return *this;
		}
		YourJsonData& operator=(const bool b) {
			value = b;
			return *this;
		}
		YourJsonData& operator=(const long long d) {
			value = d;
			return *this;
		}
		YourJsonData& operator=(const int d) {
			value = (long long)d;
			return *this;
		}
		YourJsonData& operator=(const double d) {
			value = d;
			return *this;
		}
		YourJsonData& operator=(const std::string& str) {
			SetStr(str);
			return *this;
		}
		YourJsonData& operator=(const char* str) {
			SetStr(str);
			return *this;
		}
		bool IsStr() const {
			return std::holds_alternative <YourJsonStrPtr>(value);
		}
		bool IsArr() const {
			return std::holds_alternative <YourJsonArrPtr>(value);
		}
		bool IsObj() const {
			return std::holds_alternative <YourJsonObjPtr>(value);
		}
		bool IsInt() const {
			return std::holds_alternative <long long>(value);
		}
		bool IsNum() const {
			return std::holds_alternative <double>(value);
		}
		bool IsBool() const {
			return std::holds_alternative <bool>(value);
		}
		bool IsNull() const {
			return std::holds_alternative <YourJsonNull>(value);
		}
		YourJsonObj* GetObjPtr() const {
			return std::get<YourJsonObjPtr>(value).get();
		}
		YourJsonArr* GetArrPtr() const {
			return std::get<YourJsonArrPtr>(value).get();
		}
		YourJsonStr* GetStrPtr() const {
			return std::get<YourJsonStrPtr>(value).get();
		}
		double GetNum() const {
			return std::get<double>(value);
		}
		long long GetInt() const {
			return std::get<long long>(value);
		}
		bool GetBool() const {
			return std::get<bool>(value);
		}


		YourJsonData* FindPtrInObj(const std::string& key) {
			if (IsObj()) {
				auto ptr = GetObjPtr();
				if (ptr->find(key) == ptr->end()) return nullptr;
				return &ptr->find(key)->second;
			}
			return nullptr;
		}

		bool AddToObj(const std::string& key, YourJsonData&& val) {
			if (IsObj()) {
				GetObjPtr()->emplace(key, std::move(val));
				return true;
			}
			return false;
		}
		bool AddToArr(YourJsonData&& val) {
			if (IsArr()) {
				GetArrPtr()->emplace_back(std::move(val));
				return false;
			}
			return true;
		}

		void NewObj(size_t capa) {
			auto obj = std::make_unique<YourJsonObj>();
			obj->reserve(capa);
			value = std::move(obj);
		}
		void NewArr(size_t capa) {
			auto arr =  std::make_unique<YourJsonArr>();
			arr->reserve(capa);
			value = std::move(arr);
		}
		void DumpJson(const YourJsonData& data, bool notab=false) const {
			static int ntab = 0;
			if (&data == this) ntab = 0; 
			if (!notab) {
				for (int i = 0; i < ntab; i++) {
					//putchar('  ');
				}
				ntab++;
			}
			if (data.IsObj()) {
				putchar('{');
				putchar('\n');
				
				
				for (auto& map : *data.GetObjPtr()) {
					for (int i = 0; i < ntab; i++) {
						//putchar('  ');
					}
					printf("%s", map.first.c_str());
					putchar(':');
					DumpJson(map.second, true);
					putchar(',');
					putchar('\n');
				}
				for (int i = 0; i < ntab; i++) {
					//putchar('  ');
				}
				putchar('}');
				putchar('\n');
			}
			else if (data.IsArr()) {
				putchar('[');
				for (auto& arr : *data.GetArrPtr()) {
					DumpJson(arr, true);
					putchar(',');
				}
				putchar(']');
			}
			else if (data.IsStr()) {
				printf("%s", data.GetStrPtr()->c_str());
			}
			else if (data.IsInt()) {
				printf("%lld", data.GetInt());
			}
			else if (data.IsBool()) {
				printf("%s", data.GetBool()?"true":"false");
			}
			else if (data.IsNum()) {
				printf("%lf", data.GetNum());
			}
			else if (data.IsNull()) {
				printf("null");
			}
		}
		void Dump() {
			DumpJson(*this);
		}
		void DumpString() {
		}
		void Delete(){}
	};
};
void myjson_test() {
	ug::YourJsonData js;
	ug::YourJsonData buf = 1ll;
	js.AddToArr(std::move(buf));
	buf = 300ll;
	buf = "fhit";
	js = "Shut up";
	js.NewObj(5);
	js.AddToObj("hello", 100);
	js.AddToObj("you", 100.351);
	js.AddToObj("dirty", ug::YourJsonNull());
	{
		auto ptr = js.FindPtrInObj("hello");
		ptr->NewObj(5);
		ptr->AddToObj("Haha", "Sturing");
		ptr->AddToObj("Helly", 3.1415926535);
		ptr->AddToObj("Boo!!!", true);
		js.DumpJson(js);


		ptr->NewArr(3);
		for(int i=0;i<100;i++)
			ptr->AddToArr(i);
		js.DumpJson(js);
	}
	
	

#ifdef _WIN32
	//SetConsoleOutputCP(CP_UTF8);
	//SetConsoleCP(65001);
#endif
	ug::MyJsonParser<ug::MyJsonData>::InitMyJsonParser();

	ug::MyJsonParser<ug::MyJsonData> json;
	if (!json.NewFromFile("test.json")) {
		puts("Failed to open");
		return;
	}

	if (json.Parse()) {
		std::string str;
		//json.DumpString(str);
		json.Dump();
		//PrintUtf8(str);
		{
			FILE* fp;
			fopen_s(&fp, "export.json", "wb");
			fwrite(&str[0], 1, str.size(), fp);
			fclose(fp);
		}
	}
	else {
		puts("Failed to Parse");
		json.Dump();
	}
	json.Delete();
	ug::MyJsonParser<ug::YourJsonData>::DeleteMyJsonParser();
}
