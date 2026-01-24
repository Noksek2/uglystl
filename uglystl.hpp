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
	static uint32_t make_hash(const char* key, uint32_t length) {
		if (length == 0u) length = strlen(key);
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
		const Iter<T>& ToIter() { return Iter<T>(*this); }
	};
	
	TMPKV struct HashPair {
		K key;
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
		const Iter& ToIter() {
			return *this;
		}

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
