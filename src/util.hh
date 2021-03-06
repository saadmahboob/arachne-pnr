/* Copyright (C) 2015 Cotton Seed
   
   This file is part of arachne-pnr.  Arachne-pnr is free software;
   you can redistribute it and/or modify it under the terms of the GNU
   General Public License version 2 as published by the Free Software
   Foundation.
   
   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>. */

#ifndef PNR_UTIL_HH
#define PNR_UTIL_HH

#include <functional>
#include <iterator>
#include <algorithm>
#include <sstream>
#include <ostream>
#include <string>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <vector>
#include <random>
#include <type_traits>

#include <cassert>

extern std::ostream *logs;

template<class T> std::ostream &
operator<<(std::ostream &s, const std::set<T> &S)
{
  s << "{";
  std::copy(S.begin(), S.end(), std::ostream_iterator<T>(s, ", "));
  return s << "}";
}

template<class T> std::ostream &
operator<<(std::ostream &s, const std::unordered_set<T> &S)
{
  s << "{";
  std::copy(S.begin(), S.end(), std::ostream_iterator<T>(s, ", "));
  return s << "}";
}

template<typename K, typename V> struct PrettyKV
{
  const std::pair<K, V> &p;
  PrettyKV(const std::pair<K, V> &p_) : p(p_) {}
};

template<typename K, typename V> inline std::ostream &
operator<<(std::ostream &s, const PrettyKV<K, V> &x)
{
  return s << x.p.first << ": " << x.p.second << "\n";
}

template<typename K, typename V> std::ostream &
operator<<(std::ostream &s, const std::map<K, V> &M)
{
  s << "{";
  std::transform(M.begin(), M.end(),
		 std::ostream_iterator<std::string>(s, ", "),
		 [](const std::pair<K, V> &p) { return PrettyKV<K, V>(p); });
  return s << "}";
}

template<typename K, typename V> std::ostream &
operator<<(std::ostream &s, const std::unordered_map<K, V> &M)
{
  s << "{";
  std::transform(M.begin(), M.end(),
		 std::ostream_iterator<std::string>(s, ", "),
		 [](const std::pair<K, V> &p) { return PrettyKV<K, V>(p); });
  return s << "}";
}

#define fmt(x) (static_cast<const std::ostringstream&>(std::ostringstream() << x).str())

extern void fatal(const std::string &msg);
extern void warning(const std::string &msg);

template<typename S, typename T> void
extend(S &s, const T &x)
{
  assert(s.find(x) == s.end());
  s.insert(x);
}

template<typename S, typename T> inline bool
contains(const S &s, const T &x)
{
  return s.find(x) != s.end();
}

template<typename M, typename K, typename V> inline void
extend(M &m, const K &k, const V &v)
{
  assert(m.find(k) == m.end());
  m.insert(std::make_pair(k, v));
}

template<typename M, typename K> inline bool
contains_key(const M &m, const K &k)
{
  return m.find(k) != m.end();
}

template<typename M> inline std::set<typename M::key_type>
keys(const M &m)
{
  std::set<typename M::key_type> keys;
  std::transform(m.begin(), m.end(),
		 std::inserter(keys, keys.end()),
		 [](const typename M::value_type &p) { return p.first; });
  return std::move(keys);
}

template<typename M> inline std::unordered_set<typename M::key_type>
unordered_keys(const M &m)
{
  std::unordered_set<typename M::key_type> keys;
  std::transform(m.begin(), m.end(),
		 std::inserter(keys, keys.end()),
		 [](const typename M::value_type &p) { return p.first; });
  return std::move(keys);
}

extern std::string unescape(const std::string &s);

template<typename K, typename V> inline const V &
lookup(const std::map<K, V> &M, const K &key)
{
  return M.at(key);
}

template<typename M, typename K, typename V> inline typename M::mapped_type
lookup_or_default(const M &m, const K &key, const V &def)
{
  auto i = m.find(key);
  if (i != m.end())
    return i->second;
  else
    return def;
}

template<typename M, typename K, typename F> inline typename M::mapped_type &
lookup_or_create(M &m, const K &key, const F &f)
{
  auto i = m.find(key);
  if (i == m.end())
    i = m.insert(std::make_pair(key, f())).first;
  return i->second;
}

template<typename C> inline typename C::const_reference
front(const C &c)
{
  assert(c.begin() != c.end());
  return *c.begin();
}

inline bool
is_prefix(const std::string &s1, const std::string &s2)
{
  auto r = std::mismatch(s1.begin(), s1.end(), 
			 s2.begin(),
			 std::equal_to<char>());
  return r.first == s1.end();
}

extern std::string proc_self_dirname();

inline char
hexdigit(int i, char a = 'a')
{
  assert(i >= 0 && i < 16);
  return (i < 10 
	  ? '0' + i
	  : a + (i - 10));
}

template<typename T, typename RG> inline const T &
random_element(const std::vector<T> &v, RG &rg)
{
  return v[std::uniform_int_distribution<int>(0, v.size() - 1)(rg)];
}

template<typename RG> inline int
random_int(int min, int max, RG &rg)
{
  assert(min <= max);
  return std::uniform_int_distribution<int>(min, max)(rg);
}

template<typename T> inline std::size_t
hash_combine(std::size_t h, const T &v)
{
  std::hash<T> hasher;
  return h ^ (hasher(v) + 0x9e3779b9 + (h << 6) + (h >> 2));
}

namespace std {

template<typename S, typename T>
struct hash<pair<S, T>>
{
public:
  size_t operator() (const pair<S, T> &p) const
  {
    hash<int> hasher;
    size_t h = hasher(p.first);
    return hash_combine(h, hasher(p.second));
  }
};

}

extern std::string expand_filename(const std::string &file);

#endif
