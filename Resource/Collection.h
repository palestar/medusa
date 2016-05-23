/*
	Collection.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef RESOURCE_COLLECTION_H
#define RESOURCE_COLLECTION_H

#include "Standard/Array.h"
#include "Resource.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

class DLL Collection : public Resource
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef ResourceLink<Collection>		Link;

	// Constructions
	Collection();

	// Accessors
	const ClassKey &	type() const;								// type of resource contained in this collection
	int					resourceCount() const;
	Resource::Link		resource(int n) const;
	
	// Mutators
	void				setType( const ClassKey & type );			// set type of class contained in this collection
	bool				push( Resource::Link pResource );				// add to end of array
	bool				push( Collection * pCollection );			// add collection to this collection
	void				replace( int n, Resource::Link pResource );		// replace entry
	void				remove( int resource );						// remove from array

	void				load();										// load all resources
	void				unload();									// unload all resources
	void				release();									// release entire array

private:
	// Types
	typedef Array< Resource::Link >		ResourceArray;

	// Data
	ClassKey			m_Type;								
	ResourceArray		m_Resources;
};

//----------------------------------------------------------------------------

inline const ClassKey & Collection::type() const
{
	return m_Type;
}

inline int Collection::resourceCount() const
{
	return m_Resources.size();
}

inline Resource::Link Collection::resource(int n) const
{
	return m_Resources[n];
}


//----------------------------------------------------------------------------



#endif 

//-------------------------------------------------------------------------------
// EOF
