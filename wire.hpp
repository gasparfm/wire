/*
 * Extended C++ standard string classes, string interpolation and casting macros.
 * Copyright (c) 2010-2013, Mario 'rlyeh' Rodriguez

 * wire::eval() based on code by Peter Kankowski (see http://goo.gl/Kx6Oi)
 * wire::format() based on code by Adam Rosenfield (see http://goo.gl/XPnoe)
 * wire::format() based on code by Tom Distler (see http://goo.gl/KPT66)

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.

 * @todo:
 * - string::replace_map(): specialize for target_t == char || replacement_t == char
 * - string::replace_map(): specialize for (typename<size_t N> const char (&from)[N], const char (&to)[N])
 * - strings::subset( 0, EOF )
 * - strings::subset( N, EOF )
 * - strings::subset( 0, -1 ) -> 0, EOF - 1
 * - strings::subset( -2, -1 ) -> EOF-2, EOF-1

 * - rlyeh
 */

#pragma once

#include <cstdarg>
#include <cstring>

#include <algorithm>
#include <cctype>
#include <deque>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace wire
{
    /* Public API */
    // Function tools

    // Function to do safe C-style formatting
    std::string format( const char *fmt, ... );

    // Function to convert strings <-> numbers in most precise way (C99)
    static inline std::string precise( const long double &t ) {
        /**/ if( t ==  std::numeric_limits< long double >::infinity() ) return  "INF";
        else if( t == -std::numeric_limits< long double >::infinity() ) return "-INF";
        else if( t != t ) return "NaN";
        // C99 way {
        char buf[ 32 ];
        sprintf(buf, "%a", t);
        return buf;
        // }
    }
    static inline long double precise( const std::string &t ) {
        long double ld;
        sscanf(t.c_str(), "%la", &ld);
        return ld;
    }

    // Function to evaluate simple numeric expressions
    double eval( const std::string &expression );

    /* Public API */
    // Main class

    namespace
    {
        template< typename T >
        inline T as( const std::string &self ) {
            T t;
            if( std::istringstream(self) >> t )
                return t;
            bool is_true = self.size() && (self != "0") && (self != "false");
            return (T)(is_true);
        }

        template<>
        inline char as( const std::string &self ) {
            return self.size() == 1 ? (char)(self[0]) : (char)(as<int>(self));
        }
        template<>
        inline signed char as( const std::string &self ) {
            return self.size() == 1 ? (signed char)(self[0]) : (signed char)(as<int>(self));
        }
        template<>
        inline unsigned char as( const std::string &self ) {
            return self.size() == 1 ? (unsigned char)(self[0]) : (unsigned char)(as<int>(self));
        }

        template<>
        inline const char *as( const std::string &self ) {
            return self.c_str();
        }
        template<>
        inline std::string as( const std::string &self ) {
            return self;
        }
    }

    class string : public std::string
    {
        public:

        // basic constructors

        string() : std::string()
        {}

        string( const std::string &s ) : std::string( s )
        {}

        string( const char &c ) : std::string( 1, c )
        {}

        string( const char &c, size_t n ) : std::string( n, c )
        {}

        string( size_t n, const char &c ) : std::string( n, c )
        {}

        string( const char *cstr ) : std::string( cstr ? cstr : "" )
        {}

        string( char * const &cstr ) : std::string( cstr ? cstr : "" )
        {}

        template<size_t N>
        string( const char (&cstr)[N] ) : std::string( cstr )
        {}

        string( const bool &val ) : std::string( val ? "true" : "false" )
        {}

        // constructor sugars

        template< typename T >
        string( const T &t ) : std::string()
        {
            std::stringstream ss;
            if( ss << /* std::boolalpha << */ t )
                this->assign( ss.str() );
        }

        string( const float &t ) : std::string()
        {
            *this = string( (long double)t );
        }

        string( const double &t ) : std::string()
        {
            *this = string( (long double)t );
        }

        string( const long double &t ) : std::string()
        {
            // enum { max_digits = std::numeric_limits<long double>::digits10 + 2 };
            std::stringstream ss;
            if( ss << (long double)(t) )
                this->assign( ss.str() );
        }

        // extended constructors; safe formatting

        private:
        template<unsigned N>
        std::string &formatsafe( const std::string &fmt, std::string (&t)[N] )
        {
            for( const unsigned char &ch : fmt ) {
                if( ch > N ) t[0] += char(ch);
                else t[0] += t[ ch ];
            }
            return t[0];
        }
        public:

        template< typename T1 >
        string( const std::string &fmt, const T1 &t1 ) : std::string()
        {
            std::string t[] = { std::string(), string(t1) };
            assign( formatsafe( fmt, t ) );
        }

        template< typename T1, typename T2 >
        string( const std::string &fmt, const T1 &t1, const T2 &t2 ) : std::string()
        {
            std::string t[] = { std::string(), string(t1), string(t2) };
            assign( formatsafe( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3 >
        string( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3 ) : std::string()
        {
            std::string t[] = { std::string(), string(t1), string(t2), string(t3) };
            assign( formatsafe( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3, typename T4 >
        string( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4 ) : std::string()
        {
            std::string t[] = { std::string(), string(t1), string(t2), string(t3), string(t4) };
            assign( formatsafe( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5 >
        string( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5 ) : std::string()
        {
            std::string t[] = { std::string(), string(t1), string(t2), string(t3), string(t4), string(t5) };
            assign( formatsafe( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5, typename T6 >
        string( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6 ) : std::string()
        {
            std::string t[] = { std::string(), string(t1), string(t2), string(t3), string(t4), string(t5), string(t6) };
            assign( formatsafe( fmt, t ) );
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7 >
        string( const std::string &fmt, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7 ) : std::string()
        {
            std::string t[] = { std::string(), string(t1), string(t2), string(t3), string(t4), string(t5), string(t6), string(t7) };
            assign( formatsafe( fmt, t ) );
        }

        wire::string &operator()() {
            return *this;
        }

        template< typename T1 >
        wire::string &operator()( const T1 &t1 ) {
            return assign( string( *this, t1 ) ), *this;
        }

        template< typename T1, typename T2 >
        wire::string &operator()( const T1 &t1, const T2 &t2 ) {
            return assign( string( *this, t1, t2 ) ), *this;
        }

        template< typename T1, typename T2, typename T3 >
        wire::string &operator()( const T1 &t1, const T2 &t2, const T3 &t3 ) {
            return assign( string( *this, t1, t2, t3 ) ), *this;
        }

        template< typename T1, typename T2, typename T3, typename T4 >
        wire::string &operator()( const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4 ) {
            return assign( string( *this, t1, t2, t3, t4 ) ), *this;
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5 >
        wire::string &operator()( const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5 ) {
            return assign( string( *this, t1, t2, t3, t4, t5 ) ), *this;
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5, typename T6 >
        wire::string &operator()( const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6 ) {
            return assign( string( *this, t1, t2, t3, t4, t5, t6 ) ), *this;
        }

        template< typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7 >
        wire::string &operator()( const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7 ) {
            return assign( string( *this, t1, t2, t3, t4, t5, t6, t7 ) ), *this;
        }

        // conversion

        template< typename T >
        T as() const
        {
            return wire::as<T>(*this);
        }

        template< typename T >
        operator T() const
        {
            return wire::as<T>(*this);
        }

        // chaining operators

        template <typename T>
        string &operator <<( const T &t )
        {
            //*this = *this + string(t);
            this->append( string(t) );
            return *this;
        }

        string &operator <<( std::ostream &( *pf )(std::ostream &) )
        {
            return *pf == static_cast<std::ostream& ( * )(std::ostream&)>( std::endl ) ? (*this) += "\n", *this : *this;
        }

        template< typename T >
        string &operator +=( const T &t )
        {
            return operator<<(t);
        }

        string &operator +=( std::ostream &( *pf )(std::ostream &) )
        {
            return operator<<(pf);
        }

        // assignment sugars

        template< typename T >
        string &operator=( const T &t )
        {
            this->assign( string(t) );
            return *this;
        }

        // comparison sugars
/*
        operator const bool() const
        {
            return wire::as<bool>(*this);
        }
*/
        template<typename T>
        bool operator ==( const T &t ) const
        {
            return wire::as<T>(*this) == wire::string(t).as<T>();
        }
        bool operator ==( const wire::string &t ) const
        {
            return this->compare( t ) == 0;
        }
        bool operator ==( const char *t ) const
        {
            return this->compare( t ) == 0;
        }

        // extra methods

        // at() classic behaviour: "hello"[5] = undefined, "hello"[-1] = undefined
        // at() extended behaviour: "hello"[5] = h, "hello"[-1] = o,

        const char &at( const int &pos ) const
        {
            signed size = (signed)(this->size());
            if( size )
                return this->std::string::at( pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) );
            static std::map< const string *, char > map;
            return ( ( map[ this ] = map[ this ] ) = '\0' );
        }

        char &at( const int &pos )
        {
            signed size = (signed)(this->size());
            if( size )
                return this->std::string::at( pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) );
            static std::map< const string *, char > map;
            return ( ( map[ this ] = map[ this ] ) = '\0' );
        }

        const char &operator[]( const int &pos ) const {
            return this->at(pos);
        }
        char &operator[]( const int &pos ) {
            return this->at(pos);
        }

        void pop_back()
        {
            if( this->size() )
                this->erase( this->end() - 1 );
        }

        void pop_front()
        {
            if( this->size() )
                this->erase( 0, 1 ); //this->substr( 1 ); //this->assign( this->begin() + 1, this->end() );
        }

        template<typename T>
        void push_back( const T& t ) {
            //std::swap( *this, *this + string(t) );
            *this = *this + string(t);
        }

        template<typename T>
        void push_front( const T& t ) {
            //std::swap( *this, string(t) + *this );
            *this = string(t) + *this;
        }

        const char &back() const
        {
            return at(-1);
        }

        char &back()
        {
            return at(-1);
        }

        const char &front() const
        {
            return at(0);
        }

        char &front()
        {
            return at(0);
        }

        // tools

        std::string str( const std::string &pre = std::string(), const std::string &post = std::string() ) const
        {
            return pre + *this + post;
        }

        string uppercase() const
        {
            std::string s = *this;

            std::transform( s.begin(), s.end(), s.begin(), (int(*)(int)) std::toupper );

            return s;
        }

        string lowercase() const
        {
            std::string s = *this;

            std::transform( s.begin(), s.end(), s.begin(), (int(*)(int)) std::tolower );

            return s;
        }

        bool matches( const std::string &pattern ) const
        {
            struct local {
                static bool match( const char *pattern, const char *str ) {
                    if( *pattern=='\0' ) return !*str;
                    if( *pattern=='*' )  return match(pattern+1, str) || *str && match(pattern, str+1);
                    if( *pattern=='?' )  return *str && (*str != '.') && match(pattern+1, str+1);
                    return (*str == *pattern) && match(pattern+1, str+1);
                }
            };

            return local::match( pattern.c_str(), (*this).c_str() );
        }

        bool matchesi( const std::string &pattern ) const
        {
            return this->uppercase().matches( string(pattern).uppercase() );
        }

        size_t count( const std::string &substr ) const
        {
            size_t n = 0;
            std::string::size_type pos = 0;
            while( (pos = this->find( substr, pos )) != std::string::npos ) {
                n++;
                pos += substr.size();
            }
            return n;
        }

        string left_of( const std::string &substring ) const
        {
            string::size_type pos = this->find( substring );
            return pos == std::string::npos ? *this : (string)this->substr(0, pos);
        }

        string right_of( const std::string &substring ) const
        {
            std::string::size_type pos = this->find( substring );
            return pos == std::string::npos ? *this : (string)this->substr(pos + 1);
        }

        string replace1( const std::string &target, const std::string &replacement ) const {
            std::string str = *this;
            auto found = str.find(target);
            return found == string::npos ? str : (str.replace(found, target.length(), replacement), str);
        }

        string replace( const std::string &target, const std::string &replacement ) const
        {
            size_t found = 0;
            std::string s = *this;

            while( ( found = s.find( target, found ) ) != string::npos )
            {
                s.replace( found, target.length(), replacement );
                found += replacement.length();
            }

            return s;
        }

        string replace_map( const std::map< std::string, std::string > &replacements ) const
        {
            string out;

            for( size_t i = 0; i < this->size(); )
            {
                bool found = false;
                size_t match_length = 0;

                std::map< std::string, std::string >::const_reverse_iterator it;
                for( it = replacements.rbegin(); !found && it != replacements.rend(); ++it )
                {
                    const std::string &target = it->first;
                    const std::string &replacement = it->second;

                    if( match_length != target.size() )
                        match_length = target.size();

                    if( this->size() - i >= target.size() )
                    if( !std::memcmp( &this->at(i), &target.at(0), match_length ) )
                    {
                        i += target.size();

                        out += replacement;

                        found = true;
                    }
                }

                if( !found )
                    out += this->at(i++);
            }

           return out;
        }

        private:

        string strip( const std::string &chars, bool strip_left, bool strip_right ) const
        {
            std::string::size_type len = this->size(), i = 0, j = len, charslen = chars.size();

            if( charslen == 0 )
            {
                if( strip_left )
                    while( i < len && std::isspace( this->operator[]( i ) ))
                        i++;

                if( strip_right && j ) {
                    do j--; while( j >= i && std::isspace( this->operator[]( j ) ));
                    j++;
                }
            }
            else
            {
                const char *sep = chars.c_str();

                if( strip_left )
                    while( i < len && std::memchr( sep, this->operator[]( i ), charslen ))
                        i++;

                if( strip_right && j ) {
                    do j--; while( j >= i && std::memchr( sep, this->operator[]( j ), charslen ));
                    j++;
                }
            }

            if( j - i == len ) return *this;
            return this->substr( i, j - i );
        }

        public: // based on python string and pystring

        // Return a copy of the string with leading characters removed (default chars: space)
        string lstrip( const std::string &chars = std::string() ) const
        {
            return strip( chars, true, false );
        }
        string ltrim( const std::string &chars = std::string() ) const
        {
            return strip( chars, true, false );
        }

        // Return a copy of the string with trailing characters removed (default chars: space)
        string rstrip( const std::string &chars = std::string() ) const
        {
            return strip( chars, false, true );
        }
        string rtrim( const std::string &chars = std::string() ) const
        {
            return strip( chars, false, true );
        }

        // Return a copy of the string with both leading and trailing characters removed (default chars: space)
        string strip( const std::string &chars = std::string() ) const
        {
            return strip( chars, true, true );
        }
        string trim( const std::string &chars = std::string() ) const
        {
            return strip( chars, true, true );
        }

        bool starts_with( const std::string &prefix ) const
        {
            return this->size() >= prefix.size() ? this->substr( 0, prefix.size() ) == prefix : false;
        }

        bool starts_withi( const std::string &prefix ) const
        {
            return this->uppercase().starts_with( string(prefix).uppercase() );
        }

        bool ends_with( const std::string &suffix ) const
        {
            return this->size() < suffix.size() ? false : this->substr( this->size() - suffix.size() ) == suffix;
        }

        bool ends_withi( const std::string &suffix ) const
        {
            return this->uppercase().ends_with( string(suffix).uppercase() );
        }

        std::deque< string > tokenize( const std::string &delimiters ) const {
            std::string map( 256, '\0' );
            for( const unsigned char &ch : delimiters )
                map[ ch ] = '\1';
            std::deque< string > tokens(1);
            for( const unsigned char &ch : *this ) {
                /**/ if( !map.at(ch)          ) tokens.back().push_back( char(ch) );
                else if( tokens.back().size() ) tokens.push_back( string() );
            }
            while( tokens.size() && !tokens.back().size() ) tokens.pop_back();
            return tokens;
        }

        // tokenize_incl_separators
        std::deque< string > split( const std::string &delimiters ) const {
            std::string str;
            std::deque< string > tokens;
            for( auto &ch : *this ) {
                if( delimiters.find_first_of( ch ) != std::string::npos ) {
                    if( str.size() ) tokens.push_back( str ), str = "";
                    tokens.push_back( std::string() + ch );
                } else str += ch;
            }
            return str.empty() ? tokens : ( tokens.push_back( str ), tokens );
        }
    };

    class strings : public std::deque< string >
    {
        public:

        strings() : std::deque< string >()
        {}

        strings( const int &argc, const char **&argv ) : std::deque< string >()
        {
            for( int i = 0; i < argc; ++i )
                this->push_back( argv[i] );
        }

        strings( const int &argc, char **&argv ) : std::deque< string >()
        {
            for( int i = 0; i < argc; ++i )
                this->push_back( argv[i] );
        }

        template< typename T, const size_t N >
        strings( const T (&args)[N] ) : std::deque< string >()
        {
            this->resize( N );
            for( int n = 0; n < N; ++n )
                (*this)[ n ] = args[ n ];
        }

        template <typename CONTAINER>
        strings( const CONTAINER &other ) : std::deque< string >( other.begin(), other.end() )
        {}

        template <typename CONTAINER>
        strings &operator =( const CONTAINER &other ) : std::deque< string >() {
            if( &other != this ) {
                *this = strings( other );
            }
            return *this;
        }

        template< typename T > strings( const T &t0, const T &t1 ) : std::deque< string >()
        { this->resize(2); (*this)[0] = t0; (*this)[1] = t1; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2 ) : std::deque< string >()
        { this->resize(3); (*this)[0] = t0; (*this)[1] = t1; (*this)[2] = t2; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2, const T &t3 ) : std::deque< string >()
        { this->resize(4); (*this)[0] = t0; (*this)[1] = t1; (*this)[2] = t2; (*this)[3] = t3; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2, const T &t3, const T &t4 ) : std::deque< string >()
        { this->resize(5); (*this)[0] = t0; (*this)[1] = t1; (*this)[2] = t2; (*this)[3] = t3; (*this)[4] = t4; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2, const T &t3, const T &t4, const T &t5 ) : std::deque< string >()
        { this->resize(6); (*this)[0] = t0; (*this)[1] = t1; (*this)[2] = t2; (*this)[3] = t3; (*this)[4] = t4; (*this)[5] = t5; }
        template< typename T > strings( const T &t0, const T &t1, const T &t2, const T &t3, const T &t4, const T &t5, const T &t6 ) : std::deque< string >()
        { this->resize(7); (*this)[0] = t0; (*this)[1] = t1; (*this)[2] = t2; (*this)[3] = t3; (*this)[4] = t4; (*this)[5] = t5; (*this)[6] = t6; }

        const string &at( const int &pos ) const
        {
            signed size = signed(this->size());
            if( size )
                return *( this->begin() + ( pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) ) );
            static std::map< const strings *, string > map;
            return ( ( map[ this ] = map[ this ] ) = string() );
        }

        string &at( const int &pos )
        {
            signed size = signed(this->size());
            if( size )
                return *( this->begin() + ( pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) ) );
            static std::map< const strings *, string > map;
            return ( ( map[ this ] = map[ this ] ) = string() );
        }

        std::string str( const char *format1 = "\1\n", const std::string &pre = std::string(), const std::string &post = std::string() ) const
        {
            if( this->size() == 1 )
                return pre + *this->begin() + post;

            std::string out( pre );

            for( const_iterator it = this->begin(); it != this->end(); ++it )
                out += string( format1, (*it) );

            return out + post;
        }

        template<typename ostream>
        inline friend ostream &operator <<( ostream &os, const wire::strings &self ) {
            return os << self.str(), self;
        }
    };
}

#include <iostream>

std::ostream &operator <<( std::ostream &os, const wire::strings &s );

// String interpolation and string casting macros. MIT licensed.

namespace wire
{
    // public api, define/update

#   define $(a)         wire::locate( "$" #a )

    // public api, translate

#   define $$(a)        wire::translate( a )

    // public api, cast and sugars

#   define $cast(a,b)   $( a ).as<b>()

#   define $string(a)   $( a )               // no need to cast
#   define $bool(a)     $cast( a, bool     )
#   define $char(a)     $cast( a, char     )
#   define $int(a)      $cast( a, int      )
#   define $float(a)    $cast( a, float    )
#   define $double(a)   $cast( a, double   )
#   define $size_t(a)   $cast( a, size_t   )
#   define $unsigned(a) $cast( a, unsigned )

    // private details

    std::vector< std::string > extract( const wire::string &dollartext, char sep0 = '$', char sep1 = '\0' );
    wire::string translate( const wire::string &dollartext, const wire::string &recursive_parent = std::string() );
    wire::string &locate( const wire::string &text );
}

//

namespace wire
{
    template<typename T>
    inline std::string str( const T& t, const std::string &format1, const std::string &pre = std::string(), const std::string &post = std::string() )
    {
        wire::string out;

        for( typename T::const_iterator it = t.begin(), end = t.end(); it != end; ++it )
            out << wire::string( format1, *it );

        return wire::string() << pre << out << post;
    }

    template<typename T>
    inline std::string str1( const T& t, const std::string &format1, const std::string &pre = std::string(), const std::string &post = std::string() )
    {
        wire::string out;

        for( typename T::const_iterator it = t.begin(), end = t.end(); it != end; ++it )
            out << wire::string( format1, it->first );

        return wire::string() << pre << out << post;
    }

    template<typename T>
    inline std::string str2( const T& t, const std::string &format1, const std::string &pre = std::string(), const std::string &post = std::string() )
    {
        wire::string out;

        for( typename T::const_iterator it = t.begin(), end = t.end(); it != end; ++it )
            out << wire::string( format1, it->second );

        return wire::string() << pre << out << post;
    }

    template<typename T>
    inline std::string str12( const T& t, const std::string &format12, const std::string &pre = std::string(), const std::string &post = std::string() )
    {
        wire::string out;

        for( typename T::const_iterator it = t.begin(), end = t.end(); it != end; ++it )
            out << wire::string( format12, it->first, it->second );

        return wire::string() << pre << out << post;
    }
}

// $wire(), introspective macro

namespace wire
{
    // @todo: eq+sep+line is a c++11 constexpr
    struct parser : public wire::string {
        parser( const wire::string &fmt, const wire::string &line = std::string() ) {
            wire::strings all = line.tokenize(", \r\n\t");
            wire::strings::iterator it, begin, end;

            typedef std::pair<std::string,std::string> pair;
            std::vector< pair > results;

            for( it = begin = all.begin(), end = all.end(); it != end; ++it ) {
                results.push_back( pair( (*it).right_of(".").right_of("->"), std::string() + char(it - begin + '\1') ) );
            }

            assign( str12(results, fmt) );
        }
    };
}

#define $wire(FMT,...) wire::parser(FMT,#__VA_ARGS__)(__VA_ARGS__)

// simple getopt replacement class. mit licensed
// - rlyeh

// this geptop class is a std::map replacement where key/value are wire::string.
// given invokation './app.out --user=me --pass=123 -h' this class delivers:
// map[0] = "./app.out", map[1] = "--user=me", map[2]="--pass=123", map[3]='-h'
// but also, map["--user"]="me", map["--pass"]="123" and also, map["-h"]=true

// .cmdline() for a print app invokation string
// .str() for pretty map printing
// .size() number of arguments (equivalent to argc), rather than std::map.size()

namespace wire {

    struct getopt : public std::map< wire::string, wire::string >
    {
        getopt()
        {}

        explicit
        getopt( int argc, const char **argv ) {
            wire::strings args( argc, argv );

            // create key=value and key= args as well
            for( auto &it : args ) {
                wire::strings tokens = it.split( "=" );

                if( tokens.size() == 3 && tokens[1] == "=" )
                    (*this)[ tokens[0] ] = tokens[2];
                else
                if( tokens.size() == 2 && tokens[1] == "=" )
                    (*this)[ tokens[0] ] = true;
                else
                if( tokens.size() == 1 && tokens[0] != argv[0] )
                    (*this)[ tokens[0] ] = true;
            }

            // create args
            while( argc-- ) {
                (*this)[ argc ] = argv[argc];
            }
        }

        size_t size() const {
            unsigned i;
            for( i = 0; has(i); ++i ) 
            {}
            return i;
        }

        bool has( const wire::string &op ) const {
            return this->find(op) != this->end();
        }

        std::string str() const {
            wire::string ss;
            for( auto &it : *this )
                ss << it.first << "=" << it.second << ',';
            return ss.str();
        }

        std::string cmdline() const {
            wire::string cmd;

            // concatenate args
            for( unsigned arg = 0, end = size(); arg < end; ++arg ) {
                cmd << this->find(arg)->second << ' ';
            }

            // remove trailing space, if needed
            if( cmd.size() )
                cmd.pop_back();

            return cmd;
        }
    };
}

// simple INI reader and writer. mit licensed
// - rlyeh

namespace wire {
    // [section]
    // key=value ;comment
    struct ini : std::map< wire::string, wire::string > {

        ini() : std::map< wire::string, wire::string >()
        {}

        bool load( const std::string &text ) {
            *this = ini();
            wire::string section;
            for( auto &line : wire::string(text).tokenize("\r\n") ) {
                // remove comments, split line into tokens and parse tokens
                line = line.substr( 0, line.find_first_of(';') );
                wire::strings t = line.split("[]=");
                /**/ if( t.size() == 3 && t[0] == "[" && t[2] == "]" ) section = t[1];
                else if( t.size() == 3 && t[1] == "=" ) (*this)[section + "." + t[0]] = t[2];
                else return false;
            }
            return true;
        }

        std::string save() const {
            std::string output( "; auto-generated by ini class\r\n" ), section;
            for( auto &it : *this ) {
                wire::strings kv = it.first.tokenize(".");
                if( section != kv[0] ) {
                    output += "\r\n[" + ( section = kv[0] ) + "]\r\n";
                }
                output += kv[1] + "=" + it.second + "\r\n";
            }
            return output;
        }
    };
}
