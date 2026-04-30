//////////////////////////////////////////////////////////////////////////
//  - 싱글톤 -
//  
//  
//////////////////////////////////////////////////////////////////////////
#ifndef __SINGLETON_H__
#define __SINGLETON_H__

/*+++++++++++++++++++++++++++++++++++++
    CLASS.
+++++++++++++++++++++++++++++++++++++*/
template <typename T> 
class Singleton
{
    static T* _Singleton;

public:
    Singleton( void )
    {
        if ( _Singleton==0 )
        {
            ptrdiff_t offset = (reinterpret_cast<char*>(static_cast<T*>((void*)1)) - reinterpret_cast<char*>(static_cast<Singleton<T>*>(static_cast<T*>((void*)1))));
            _Singleton = reinterpret_cast<T*>(reinterpret_cast<char*>(this) + offset);
        }
    }
    
    virtual ~Singleton( void ) {  /*assert( _Singleton );*/  _Singleton = 0;  }
    
    static T&   GetSingleton ( void )      {  /*assert( _Singleton );*/  return ( *_Singleton );  }
    static T*   GetSingletonPtr ( void )   {  return ( _Singleton ); } 
	static bool IsInitialized ( void )     { return _Singleton ? true : false; }

	//여기 부분은 좀 생각을 해보자..
	//new로 만들어서 넣으면...delete를 해줘야 하는데...
	//할려면 boost로 만들어진 data만 넣도록 하자.
	//static void RegisterSingleton ( T* p ) { _Singleton = p; }
};

template <typename T> T* Singleton <T>::_Singleton = 0;

#endif