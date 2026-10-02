#pragma once
//#include <iostream>
//#include <string>
//#include <vector>
//#include <variant>
//#include <optional>
//#include <type_traits>
//#include <fstream>
//#include <filesystem>
#include <stdint.h>
#include <stdio.h>
#if _WIN32
	#include <Windows.h>
#else
#endif
// RAII is LIMITED
// MOVE 
//virtual keyword
namespace uglystd {
	//ALL Object is Data
	//Raw : NoRAII
	enum Error {
		err_none,
		err_Base_,
		
		err_Memory_ReserveFailed,
		err_Memory_CommitFailed,
		err_Memory_NotEmpty,
		err_Memory_ReleaseFailed,

		err_Vector_OutOfIndex,
		err_Vector_Empty,
		err_Vector_DataIsNull,
		err_Vector_Full,
		err_Vector_ReserveFailed,
	};

	
	class ErrorStack {//처리되지 않음.
	private:
		static constexpr uint32_t ErrorStack_MAX = 256;
		static thread_local Error stack[ErrorStack_MAX];
		static uint32_t len;
	public:
		static void Push(Error err) {
			if (err == err_none) return;
			if (len == ErrorStack_MAX) return;
			stack[len++] = err;
		}
		static Error Pop() {
			if(len) return stack[--len];
			return err_none;
		}
		static uint32_t Size() {
			return len;
		}
	};
	
	
	class _Base {
	protected:
		_Base() noexcept = default;
		~_Base() = default;
	public:
		_Base(_Base&&) = delete;
		_Base(const _Base&) = delete;
		_Base& operator=(const _Base&) = delete;
		_Base& operator=(_Base&&) noexcept = delete;
	};
	class Base : private _Base{
		unsigned int nProc : 7;
		unsigned int isMoved: 1;
	public:
		bool setMoved() { 
			if(isMoved) return false; 
			isMoved = true;
			return true;
		}
		bool checkMoved() {
			return isMoved;
		}
		bool incProc() {
			if (nProc < (1 << 6u)) nProc++; 
			return false;
		}
		bool checkProc(uint32_t nTargetProc) {
			return nProc == nTargetProc;
		}
	};
	//template<class T>
	struct MemoryPage {
		void* m_reserved_mem;
		size_t m_commited;
		size_t m_reserved;

		Error Init() {
			if (m_reserved_mem)
				return err_Memory_NotEmpty;
			m_reserved_mem = nullptr;
			m_commited = 0;
			m_reserved = 0;
			return err_none;
		}
		Error Reserve(size_t capa) {
			if (m_reserved_mem)
				return err_Memory_NotEmpty;
			void* p;
			p = VirtualAlloc(NULL, capa, MEM_RESERVE, PAGE_READWRITE);
			if (p == NULL) {
				return err_Memory_ReserveFailed;
			}
			m_reserved_mem = p;
			m_reserved = capa;
			return err_none;
		}
		Error Commit(void*& P, size_t capa) {
			P = nullptr;
			uint8_t* _p = reinterpret_cast<uint8_t*>(m_reserved_mem);
			_p += m_commited;
			if (m_commited + capa > m_reserved)
				return err_Memory_CommitFailed;
			m_commited += capa;
			void* p = VirtualAlloc(m_reserved_mem, m_commited, MEM_COMMIT, PAGE_READWRITE);
			if (p == nullptr)
				return err_Memory_CommitFailed;
			
			P = _p;
			return err_none;
		}
		Error Release() {
			if(VirtualFree(m_reserved_mem, 0, MEM_RELEASE) == TRUE)
				return err_none;
			return err_Memory_ReleaseFailed;
		}
	};
	class Memory : Base {
		Memory* next;
		MemoryPage page;
		Memory(size_t capa) {
			next = nullptr;
			ErrorStack::Push(page.Init());
			ErrorStack::Push(page.Reserve(capa));
		}
		~Memory() {
			ErrorStack::Push(page.Release());
		}
		
	public:
		static Memory* g_mem;
		static Memory* GetGlobalMem() {
			return g_mem;
		}
		static Memory* NewGlobalMem() {
			if (g_mem == nullptr) {
				g_mem = Memory::New();
			}
			return g_mem;
		}
		static void DeleteGlobalMem() {
			if (g_mem == nullptr) return;
			delete g_mem;
			g_mem = nullptr;
		}
		static Memory* New(size_t capa = 4096) {
			Memory* newMemory = new Memory(capa);
			if (newMemory == nullptr)
				return nullptr;
			return newMemory;
		}
		template <typename T>
		Error Alloc(T*& newmem, size_t len) {
			Error err;
			void* newmem_p;
			err = page.Commit(newmem_p, len * sizeof(T));
			if (err == err_none) {
				newmem = reinterpret_cast<T*>(newmem_p);
			}
			return err;
		}
		
		Error Free(void* p) {
			puts("Memory::Free 구현 안 됨");
			return err_none;
		}
	};
	template<class T>
	class Vector : Base {
		T* m_data = nullptr;
		size_t m_len = 0;
		size_t m_capa = 0;
		Memory* MEM;
	public:
		//Vector() noexcept = default;
		Vector(Memory* Mem = Memory::GetGlobalMem()) noexcept {
			MEM = Mem;
		}
		Vector(Vector&& vec) noexcept {
			_Move(vec);
		}
		Vector& operator=(Vector&& vec) noexcept {
			_Move(vec);
			return *this;
		}
		Error Reserve(size_t capa) {
			if (m_data != nullptr) {
				return err_Vector_ReserveFailed;
			}
			MEM->Alloc<T>(m_data, capa);
			if (m_data == nullptr) return err_Vector_ReserveFailed;
			m_capa = capa;
			return err_none;
		}
		//not automatically Realloced
		Error Push(const T& val) {
			if (m_data == nullptr) return err_Vector_DataIsNull;
			if (m_len >= m_capa) return err_Vector_Full;
			m_data[m_len++] = val;
			return err_none;
		}
		Error Back(T& res) {
			if (m_len == 0u) return err_Vector_Empty;
			res = m_data[m_len - 1];
			return err_none;
		}
		Error Pop(T& res) {
			if (m_len == 0u) return err_Vector_Empty;
			res = m_data[--m_len];
			return err_none;
		}
		Error At(T& res, const size_t idx) {
			if (idx >= m_len) {
				return err_Vector_OutOfIndex;
			}
			res = m_data[idx];
			return err_none;
		}
		~Vector() {
			if(!checkMoved())
				_Destroy();
		}
	private:
		void _Destroy() {
			MEM->Free(m_data);
			m_data = nullptr;
			m_capa = 0;
			m_len = 0;
		}
		void _Move(Vector&& o) {
			m_capa = o.m_capa;
			m_len = o.m_len;
			m_data = o.m_data;
			o.setMoved();
		}
	};

	class Context {
	public:
		static void Init() {
			Memory::NewGlobalMem();
		}
		static void Delete() {
			Memory::DeleteGlobalMem();
			if (ErrorStack::Size()) {
				puts("ERROR");
				while (true) {
					Error ce = ErrorStack::Pop();
					if (ce == err_none) break;
					printf("%d ", ce);
				}
			}
		}
	};
};
namespace ug {
	using namespace uglystd;
}



#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#define TMP template <typename T>
#define TMPKV template <typename K, typename V>
#ifdef _DEBUG
	#define CHK 1
#else 
	#define CHK 0
#endif
//#define NORAII

namespace uglystl {
	typedef uint32_t mysize;

	class Random {
		uint64_t m_state;
	public:
		Random(uint64_t seed = 12345u) :m_state(seed) {}
		/*unsigned int Next() {
			unsigned int x = m_state;
			x ^= x << 13;
			x ^= x >> 17;
			x ^= x << 5;
			return m_state = x;
		}*/
		TMP T Range(const T min, const T max) {
			if (min > max) return min;
			return (T)(Next() % (max - min + 1)) + min;
		}

		uint64_t Next() {
			uint64_t z = (m_state += 0x9E3779B97F4A7C15ULL);
			z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
			z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
			return z ^ (z >> 31);
		}
	};

	template<typename T>
	static uint32_t make_hash(T key, uint32_t length) {
		if (length == 0u) length = sizeof(T);
		const char* keys = (const char*)&key;
		uint32_t hash = 2166136261u; // FNV offset basis
		for (uint32_t i = 0; i < length; i++) {
			hash ^= (uint8_t)keys[i];
			hash *= 16777619; // FNV prime
		}
		return hash;
	}

	template<>
	static uint32_t make_hash(const char* key, uint32_t length) {
		if (length == 0u) length = strlen(key) - 1;
		uint32_t hash = 2166136261u; // FNV offset basis
		for (uint32_t i = 0; i < length; i++) {
			hash ^= (uint8_t)key[i];
			hash *= 16777619; // FNV prime
		}
		return hash;
	}
	template<>
	static uint32_t make_hash(const wchar_t* key, uint32_t length) {
		if (length == 0u) length = wcslen(key);
		uint32_t hash = 2166136261u; // FNV offset basis

		for (uint32_t i = 0; i < length; i++) {
			hash ^= (uint8_t)key[i];
			hash *= 16777619; // FNV prime
		}
		return hash;
	}

	TMP class Array {

	};
	TMP class Vec {
		T* m_data;
		mysize m_len;
		mysize m_capa;

	private:
		inline void _Realloc() {
			if (m_len < m_capa) return;
			while (m_len >= m_capa) {
				m_capa = m_capa << 1;
			}
			puts("realloc");

			T* buf = (T*)realloc(m_data, sizeof(T) * m_capa);
			if (buf == nullptr) puts("error!");
			m_data = buf;
			//memcpy_s(buf, sizeof(T) * m_capa, m_data, sizeof(T) * m_len);
		}
	public:
		const mysize GetLen() { return m_len; }
		const mysize GetSize() { return m_len; }
		const mysize GetCapa() { return m_capa; }

		const T& operator[](mysize idx) {
#if CHK
			if (idx >= m_len) puts("error!");
#endif
			return m_data[idx];
		}
		const T& At(mysize idx) {
			return m_data[idx];
		}
#ifdef NORAII
#else
		Vec(mysize len, mysize capa) {
			Vec::New(len, capa);
		}
		~Vec() {
			Vec::Destroy();
		}
#endif
		void New(mysize len, mysize capa) {
			puts("new");
			m_data = (T*)malloc(sizeof(T) * capa);
			m_len = len;
			m_capa = capa;

			if (m_len > 0u) memset(m_data, 0, sizeof(T) * len);
		}
		void Destroy() {
			if (m_data) {
				puts("del");
				for (mysize i = 0u; i < m_len; i++)
					m_data[i].~T();
				free(m_data);
				m_data = nullptr;
				m_len = 0u;
				m_capa = 0u;
			}
		}
		void Add(T&& data) {
			_Realloc();
			new (&m_data[m_len]) T(data);
			m_len++;
		}
		T* AddNext(mysize addlen = 1u) {
			m_len += addlen;
			_Realloc();
			//m_data[m_len++] = data;
		}
		void Add(const T& data) {
			_Realloc();
			new (&m_data[m_len]) T(data);
			m_len++;
		}
		inline void Rewind(mysize newlen = 0u) {
			m_len = newlen;
		}
		TMP class Iter {

			Vec& m_vec_ref;
			mysize idx;
		public:
			Iter(Vec& vec)
				:m_vec_ref(vec),
				idx(0u)
			{
			}
			bool IsAble() {
				return idx < m_vec_ref.m_len;
			}
			Vec& operator*() const {
				return m_vec_ref.m_data[idx];
			}
			Vec* operator->() const {
				return m_vec_ref.m_data + idx;
			}
			Iter& operator++() { idx++; return *this; }
			Iter& operator++(int) { idx++; return *this; }
			Iter& operator--() { idx--; return *this; }
		};
		const Iter<T>& ToIter() { return (*this); }
	};
	
	TMPKV struct HashPair {
		K key;
		uint32_t hash;
		V value;
	};
	TMPKV class HashMap {
		HashPair<K,V>* m_map;
		mysize m_len;
		mysize m_capa;
	public:
		class Iter;
		HashMap(mysize len,mysize capa):
			m_len(len),
			m_capa(capa)
		{
			m_map = (HashPair<K,V>*)malloc(capa * sizeof(HashPair<K, V>));
		}
		~HashMap()
		{
			if (m_map) {
				free(m_map);
				m_map = nullptr;
			}
		}
		 Iter ToIter() {
			return *this;
		}
		Iter Fuck() { return this; }
		class Iter {
		
			HashMap& m_map_ref;
			mysize idx;
		public:
			Iter(HashMap& hashmap)
				:m_map_ref(hashmap),
				idx(0u)
			{}
			bool IsAble() {
				return idx < m_map_ref.m_len;
			}
			HashPair<K, V>& operator*() const {
				return m_map_ref.m_map[idx];
			}
			HashPair<K, V>* operator->() const {
				return m_map_ref.m_map + idx;
			}
			Iter& operator++() { idx++; return *this; }
			Iter& operator++(int) { idx++; return *this; }
			Iter& operator--() {
				idx--;
				return *this;
			}
		};
	};
	class Str {
		char* m_data;
		//UTF8
#ifdef NORAII
#else
		Str() {}
		~Str() {}
#endif
	};
}

#undef TMP
