/* 
NO INITIALIZE, DESTRUCTOR


*/
#include "mystd.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <conio.h>
#define DELTRASH(N) N(const N&)=delete; N& operator=(N&&)=delete;
#define TMP template<typename T>

/*
void 
#if MY_OS_WINDOWS
	mem = (myarena*)VirtualAlloc(
		NULL,
		size + sizeof(myarena),
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE
	);
	MY_ASSERT(mem != NULL);
#elif MY_OS_LINUX
	mem = (myarena*)mmap(
		NULL,
		size + sizeof(myarena),
		PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS,
		-1,
		0
	);
	MY_ASSERT(mem != MAP_FAILED);
#else
	mem = (myarena*)calloc(size, 1);
	MY_ASSERT(mem != NULL);
#endif
	return mem;

void allocator_free(myallocator* alc) {
	myarena* next = 0;
	myarena* now = alc->head;
	while (now != NULL) {
		next = now->next;
		MY_LOG_INFO("free block id:%llu [%llX]..", now->id, (size_t)now);

	#if MY_OS_WINDOWS
		VirtualFree(now, 0, MEM_RELEASE);
	#elif MY_OS_LINUX
		munmap(now, now->capa + sizeof(myarena));
	#else
		free(now);
	#endif
		now = next;
	}
	alc->head = 0;
	alc->current = 0;
}
*/
#define VER 1
#if VER==0
namespace nok{
	namespace Memory {
		TMP static T* Malloc(size_t len) {
			return (T*)VirtualAlloc(
				NULL,
				len * sizeof(T),
				MEM_COMMIT | MEM_RESERVE,
				PAGE_EXECUTE_READWRITE
			);
		}
		TMP void Free(T* mem) {
			VirtualFree(mem, 0, MEM_RELEASE);
		}
	}
	TMP struct Vec {
		T* data;
		size_t capa;
		size_t len;
		//DELTRASH(Vec);
		Vec() = delete;
		Vec(const Vec& v) {}
		Vec(Vec&& val) noexcept{
			data = val.data;
			capa = val.capa;
			len = val.len;
			val.Init();
		}
		Vec& operator=(const Vec& v) noexcept {
			return *this;
		}
		Vec& operator=(Vec&& v) noexcept {
			if (this != &v) {
				data = v.data;
				capa = v.capa;
				len = v.len;
				v.Init();
			}
			return *this;
		}
		~Vec() {
			if (data) {
				Vec::Free();
			}
		}
		void Init(T* data = nullptr, size_t _len = 0u, size_t _capa = 0u) {
			this->data = data;
			this->len = _len;
			this->capa = _capa;
		}
		static Vec<T> New(size_t len = 0u, size_t capa = 4u) {
			Vec<T> tmp;
			tmp.Alloc(capa);
			tmp.len = 0u;
			return tmp;
		}
		void Free() {
			if (data) {
				if constexpr (std::is_trivially_destructible_v<T>) {
				}
				else {
					for (size_t i = 0u; i < len; i++)
						data[i].~T();
				}
				puts("free!");
				nok::Memory::Free(data);
				Vec::Init();
			}
		}
		void Alloc(const size_t capa) {
			puts("Alloc!");
			this->data = nok::Memory::Malloc<T>(capa);
			this->capa = capa;
		}
		void Realloc(const size_t capa_new) {
			puts("Realloc!");
			T* tmp = nok::Memory::Malloc<T>(capa_new);
			this->capa = capa_new;
			memcpy(tmp, this->data, sizeof(T) * (this->len));
			nok::Memory::Free(this->data);
			this->data = tmp;
		}
		const T& operator[](const size_t idx) {
			return data[idx];
		}
		void Push(const T& val) {
			if (len >= capa)
			{
				Vec::Realloc(this->capa * 2);
			}
			data[len++] = val;
		}
		void Push(T&& val) {
			if (len >= capa)
			{
				Vec::Realloc(this->capa * 2);
			}
			data[len++] = std::move(val);
		}
	};
}
#elif VER==1

namespace nok {
	namespace Memory {
		TMP static T* Malloc(size_t len) {
			T* mem =
		#if MY_OS_WIN
			(T*)VirtualAlloc(
				NULL,
				len * sizeof(T),
				MEM_COMMIT | MEM_RESERVE,
				PAGE_READWRITE
			);//PAGE_EXECUTE_READWRITE
			MY_ASSERT(mem != NULL);
		#elif MY_OS_LINUX
			(T*)mmap(
				NULL,
				len * sizeof(T),
				PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS,
				-1,
				0
			);
			MY_ASSERT(mem != MAP_FAILED);
		#else
			(T*)malloc(len * sizeof(T));
			MY_ASSERT(mem != NULL);
		#endif
			
			return mem;
		}
		TMP void Free(T* mem, size_t capa=0u) {
		#if MY_OS_WIN
			VirtualFree(mem, 0, MEM_RELEASE);
		#elif MY_OS_LINUX
			munmap(now, capa * sizeof(T));
		#else
			free(mem);
		#endif
		}
	}
	TMP struct Vec;
	template <typename T> struct is_vec : std::false_type {};
	template <typename U> struct is_vec<nok::Vec<U>> : std::true_type {};

	TMP struct Vec {
		T* data;
		size_t len;
		size_t capa;

		static Vec<T> New(size_t _len, size_t _capa) {
			Vec<T> tmp;
			tmp.Init();
			tmp.Alloc(_capa);
			tmp.len = _len;
			return tmp;
		}
		void Alloc(const size_t _capa) {
			puts("Alloc!");
			data = nok::Memory::Malloc<T>(_capa);
			capa = _capa;
		}
		void Realloc(const size_t capa_new) {
			puts("Realloc!");
			T* tmp = nok::Memory::Malloc<T>(capa_new);
			this->capa = capa_new;
			memcpy(tmp, this->data, sizeof(T) * (this->len));
			nok::Memory::Free(this->data);
			this->data = tmp;
		}
		void Init() {
			data = nullptr;
			len = 0u;
			capa = 0u;
		}
		void Free() {
			if (data) {
				// 1. T가 소멸자가 필요한 놈(Vec, String 등)인지 컴파일 타임에 체크
				if constexpr (std::is_trivially_destructible_v<T>) {
					for (size_t i = 0; i < len; i++) {
						data[i].~T(); // T의 소멸자를 명시적으로 호출
					}
				}
				if constexpr (is_vec<std::decay_t<T>>::value) {
					for (size_t i = 0; i < len; i++) {
						data[i].Free(); // T의 소멸자를 명시적으로 호출
					}
					// T가 Vec<int>든 Vec<char>든 'Vec'이면 실행
				}
				puts("Free");
				// 2. 실제 메모리 해제 (C 스타일)
				nok::Memory::Free(data);

				// 3. 다시 쓸 수 있게 초기화
				this->Init();
			}
		}
		const T& operator[](const size_t idx) {
			return data[idx];
		}
		void Push(const T& val) {
			if (len >= capa) {
				Vec::Realloc(this->capa * 2);
			}
			data[len++] = val;
		}
		void Push(T&& val) {
			if (len >= capa) {
				Vec::Realloc(this->capa * 2);
			}
			data[len++] = val;
		}
		// Vec<T> Copy() {}
		// Vec<T> DeepCopy() {}
	};
}
#endif


int main() {
	
	COORD pos;
	pos.X = 10;
	pos.Y = 20;
	auto ints2 = nok::Vec<nok::Vec<int>>::New(0u,4u);
	auto buf = nok::Vec<int>::New(0u, 10u); ints2.Push(buf);
	

	ints2.Free();
	//SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);
	//SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
/*
auto v2 =v1 복사 생성자
v2 = v1 복사 대입 연산자
v2 = Vec::New(); 이동 생성자
*/