#ifndef VGSTOOLS_CACHE_H
#define VGSTOOLS_CACHE_H

#include "stdext.h"

#include <cassert>
#include <list>
#include <map>

/*  Interface for TValue:
 *  size_t weight() 
 *  bool is_newer( time_t )
 *  bool load()
 *
 */

namespace vgstools {

template< typename TKey, typename TValue, typename KeyComp = std::less< TKey > >
class cache
{
public:
	typedef TKey key_type;
	typedef TValue mapped_type;
	typedef std::tr1::shared_ptr< mapped_type > ptr_type;

private:
	typedef std::pair< key_type, ptr_type > value_type;

	typedef std::list< value_type > ValueList;
	typedef std::map< key_type, typename ValueList::iterator, KeyComp > KeyMap;

public:
	cache( size_t maxWeight )
			: m_maxWeight( maxWeight )
			, m_currentWeight( 0 ) {}

	size_t weight() const { return m_currentWeight; }
	size_t count() const { return m_values.size(); }

	ptr_type get( key_type const& key )
	{
		assert( m_lookup.size() == m_values.size() );

		typename KeyMap::iterator it = m_lookup.find( key );
		ptr_type result = it != m_lookup.end() ? it->second->second : ptr_type( new mapped_type( key ) );

		if ( it != m_lookup.end() ) {
			if ( result->is_current() ) {
				if ( it->second != m_values.begin() ) {
					m_values.erase( it->second );
					it->second = m_values.insert( m_values.begin(), std::make_pair( key, result ) );
				}
				return result;
			}

			m_currentWeight -= result->weight();
			m_values.erase( it->second );
		}

		if ( !result->load() ) {
			if ( it != m_lookup.end() )
				m_lookup.erase( it );
			return ptr_type();
		}

		// put new object into cache
		if ( result->weight() < m_maxWeight ) {
			m_currentWeight += result->weight();

			typename ValueList::iterator element = m_values.insert( m_values.begin(), std::make_pair( key, result ) );
			if ( it != m_lookup.end() )
				it->second = element;
			else
				m_lookup.insert( std::make_pair( key, element ) );

			while ( m_currentWeight > m_maxWeight ) {
				value_type& value = m_values.back();
				m_currentWeight -= value.second->weight();
				m_lookup.erase( m_lookup.find( value.first ) );
				m_values.pop_back();
			}
		}

		return result;
	}

private:
	std::size_t m_maxWeight;
	std::size_t m_currentWeight;
	ValueList m_values;
	KeyMap m_lookup;
};

} // namespace vgstools

#endif // VGSTOOLS_CACHE_H
