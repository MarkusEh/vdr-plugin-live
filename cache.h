#ifndef VGSTOOLS_CACHE_H
#define VGSTOOLS_CACHE_H

#include <algorithm>
#include <ctime>
#include <list>
#include <map>
#include <utility>
#include "stdext.h"

/*  Interface for TValue:
 *  size_t weight() 
 *  bool is_newer( time_t )
 *  bool load()
 *
 */

namespace vgstools {

template< typename TKey, typename TValue >
class cache
{
public:
	typedef TKey key_type;
	typedef TValue value_type;
	typedef std::tr1::shared_ptr< value_type > ptr_type;

private:
	struct Value
	{
		ptr_type value;
		std::time_t creation;

		Value( key_type const& key )
			: value( new value_type( key ) )
			, creation( 0 ) {}
	};

	typedef std::list< Value > ValueList;
	typedef std::map< TKey, typename ValueList::iterator > KeyMap;

public:
	cache( size_t maxWeight )
			: m_maxWeight( maxWeight )
			, m_currentWeight( 0 ) {}

	size_t weight() const { return m_currentWeight; }
	size_t count() const { return m_values.size(); }

	ptr_type get( key_type const& key )
	{
		typename KeyMap::iterator it = m_lookup.find( key );
		if ( it == m_lookup.end() ) {
			typename ValueList::iterator element = m_values.insert( m_values.begin(), Value( key ) );
			std::pair< typename KeyMap::iterator, bool > result = m_lookup.insert( std::make_pair( key, element ) );
			it = result.first;
		}

		Value* value = &*it->second;
		std::time_t now = std::time( 0 );
		if ( value->creation == 0 || !value->value->is_current() ) {
			m_currentWeight -= value->value->weight();
			if ( !value->value->load() ) {
				m_values.erase( it->second );
				m_lookup.erase( it );
				return ptr_type();
			}
			m_currentWeight += value->value->weight();
			value->creation = now;
		}
		if ( it->second != m_values.begin() ) {
			typename ValueList::iterator element = m_values.insert( m_values.begin(), *it->second );
			m_values.erase( it->second );
			it->second = element;
			value = &*element;
		}
		return value->value;
	}

private:
	std::size_t m_maxWeight;
	std::size_t m_currentWeight;
	ValueList m_values;
	KeyMap m_lookup;
};

} // namespace vgstools

#endif // VGSTOOLS_CACHE_H
