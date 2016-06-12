#ifndef I_MISC_PLY_HPP
#define I_MISC_PLY_HPP

#include<cassert>
#include<climits> /* CHAR_BIT */
#include<cstddef>
#include<iostream>
#include<limits>
#include<fstream>
#include<sstream>
#include<string>
#include<utility>
#include<vector>

#if CHAR_BIT != 8
# error 8-bit bytes assumed
#endif /* CHAR_BIT */

namespace I {

namespace misc {

/*!
 *  A class to read files in PLY (PoLYgon ?) format.
 *
 *  \code
 *      Ply ply;
 *
 *      // Read PLY header
 *      if (!ply.open(path)) {
 *          // ...
 *      }
 *
 *      // Iterate over elements
 *      for (Ply::ElementsIterator it = ply.elements_begin();
 *           it != ply.elements_end(); ++it) {
 *          const Ply::Element& element = *it;
 *
 *          if (element.name() == "some_element_to_skip") {
 *              if (!ply.skip(element))
 *                  // ...
 *              else
 *                  continue;
 *          } else {
 *              const size_type& num_elements = element.count();
 *
 *              for (size_type i = 0; i != num_elements; ++i) {
 *                  for (Ply::PropertiesIterator it2 =
 *                           element.properties_begin();
 *                      it2 = element.properties_end(); ++it2) {
 *                      const Ply::Property& property = *it2;
 *
 *                      if (it2->name() == "some_property_to_read") {
 *                          double d;
 *                          ply.read(*it2, d);
 *                         // ...
 *                      } else {
 *                          if (!ply.skip(*it2))
 *                               // ...
 *                          else
 *                              continue;
 *                      }
 *                  }
 *              }
 *          }
 *      }
 *
 *      ply.close();
 *  \endcode
 */
class Ply {
    public:
        typedef std::size_t size_type;
        typedef std::string string_type;

        /* Comments ***********************************************/

        typedef std::vector<string_type> CommentList;
        typedef CommentList::const_iterator CommentsIterator;

        /* Object informations ************************************/

        typedef std::vector<string_type> ObjInfoList;
        typedef ObjInfoList::const_iterator ObjInfosIterator;

        /* Format *************************************************/

        enum Format {
            PLY_FORMAT_ASC     = 0,
            PLY_FORMAT_BIN_BE  = 1,
            PLY_FORMAT_BIN_LE  = 2,
            PLY_FORMAT_UNKNOWN = 3,
        };

        /* Type ***************************************************/

        enum Type {
            PLY_TYPE_CHAR    = 0,
            PLY_TYPE_INT8    = PLY_TYPE_CHAR,
            PLY_TYPE_UCHAR   = 1,
            PLY_TYPE_UINT8   = PLY_TYPE_UCHAR,
            PLY_TYPE_SHORT   = 2,
            PLY_TYPE_INT16   = PLY_TYPE_SHORT,
            PLY_TYPE_USHORT  = 3,
            PLY_TYPE_UINT16  = PLY_TYPE_USHORT,
            PLY_TYPE_INT     = 4,
            PLY_TYPE_INT32   = PLY_TYPE_INT,
            PLY_TYPE_UINT    = 5,
            PLY_TYPE_UINT32  = PLY_TYPE_UINT,
            PLY_TYPE_FLOAT   = 6,
            PLY_TYPE_FLOAT32 = PLY_TYPE_FLOAT,
            PLY_TYPE_DOUBLE  = 7,
            PLY_TYPE_FLOAT64 = PLY_TYPE_DOUBLE,
            PLY_TYPE_LIST    = 8,
            PLY_TYPE_UNKNOWN = 9,
        };

        typedef char int8_type;
        typedef unsigned char uint8_type;
        typedef short int16_type;
        typedef unsigned short uint16_type;
        typedef int int32_type;
        typedef unsigned int uint32_type;
        typedef float float32_type;
        typedef double float64_type;

        /* Property ***********************************************/

        class Property {
            public:
                /*! Constructs a scalar property. */
                Property(const string_type& name, Type type) :
                    _name(name),
                    _type(type),
                    _count_type(PLY_TYPE_UNKNOWN),
                    _value_type(PLY_TYPE_UNKNOWN)
                {
                    assert(!name.empty());
                    assert(type != PLY_TYPE_LIST);
                    assert(type != PLY_TYPE_UNKNOWN);
                }

                /*! Constructs a list property. */
                Property(const string_type& name, Type count_type,
                                                  Type value_type) :
                    _name(name),
                    _type(PLY_TYPE_LIST),
                    _count_type(count_type),
                    _value_type(value_type)
                {
                    assert(!name.empty());
                    assert(count_type != PLY_TYPE_UNKNOWN);
                    assert(count_type != PLY_TYPE_LIST);
                    assert(value_type != PLY_TYPE_UNKNOWN);
                    assert(value_type != PLY_TYPE_LIST);
                }

                /*! Returns the name of the property. */
                const string_type& name() const
                {
                    return _name;
                }

                /*! Returns the type of the property. */
                Type type() const
                {
                    return _type;
                }

                /*! Returns the count type of the property (For lists only)). */
                Type count_type() const
                {
                    assert(type() == PLY_TYPE_LIST);
                    return _count_type;
                }

                /*! Returns the value type of the property (For lists only)). */
                Type value_type() const
                {
                    assert(type() == PLY_TYPE_LIST);
                    return _value_type;
                }

            protected:
                /*const*/ string_type _name;
                /*const*/ Type _type;
                /*const*/ Type _count_type; /* For lists only */
                /*const*/ Type _value_type; /* For lists only */
        };

        /* Element ************************************************/

        class Element {
            public:
                typedef std::vector<Property> PropertyList;
                typedef PropertyList::const_iterator PropertiesIterator;

            public:
                /*! Constructs an element. */
                Element(const string_type& name, size_type count) :
                    _name(name),
                    _count(count)
                {
                    /* */
                }

                /*! Returns the name of the element. */
                const string_type& name() const
                {
                    return _name;
                }

                /*! Returns the count of the element. */
                const size_type& count() const
                {
                    return _count;
                }

                /*! Returns the size required for this element
                 *       or 0 if the size is variable.
                 */
                size_type size() const
                {
                    size_type s = 0;

                    for (PropertiesIterator it = properties_begin();
                         it != properties_end(); ++it) {
                        const size_type t = type_size(it->type());

                        if (t == 0) {
                            s = 0;
                            break;
                        }

                        s += t;
                    }

                    return s;
                }

                /* Properties *************************/

                /*! Returns the number of properties of this element. */
                size_type num_properties() const
                {
                    return _properties.size();
                }

                PropertiesIterator properties_begin() const
                {
                    return _properties.begin();
                }

                PropertiesIterator properties_end() const
                {
                    return _properties.end();
                }

                /*! Adds a property to the element. */
                bool add_property(const Property& property)
                {
                    assert(property.type() != PLY_TYPE_UNKNOWN);
                    assert(!property.name().empty());

                    if (has_property(property.name())) {
                        std::cerr << "Ply: element \"" << this->name()
                                  << "\" already a property \""
                                  << property.name() << "\"" << std::endl;
                        return false;
                    }

                    _properties.push_back(property);

                    return true;
                }

                const Property& property(size_type i) const
                {
                    return _properties[i];
                }

                PropertiesIterator find_property(const string_type& name) const
                {
                    PropertiesIterator it;

                    for (it = properties_begin(); it != properties_end(); ++it)
                        if (it->name() == name)
                            return it;

                    return it; /* = properties_end() */
                }

                bool has_property(const string_type& name) const
                {
                    return (find_property(name) != properties_end());
                }

            protected:
                /* const */ string_type _name;
                /* const */ size_type _count;
                PropertyList _properties;
        };

        typedef Element::PropertiesIterator PropertiesIterator;
        typedef std::vector<Element> ElementList;
        typedef ElementList::const_iterator ElementsIterator;
 
    public:
        /*! Constructs a new Ply object. */
        Ply() :
            _format(PLY_FORMAT_UNKNOWN),
            _version(0.0)
        {
            assert(type_size(PLY_TYPE_INT8) == 1);
            assert(type_size(PLY_TYPE_UINT8) == 1);
            assert(type_size(PLY_TYPE_INT16) == 2);
            assert(type_size(PLY_TYPE_UINT16) == 2);
            assert(type_size(PLY_TYPE_INT32) == 4);
            assert(type_size(PLY_TYPE_UINT32) == 4);
            assert(type_size(PLY_TYPE_FLOAT32) == 4);
            assert(type_size(PLY_TYPE_FLOAT64) == 8);

            find_native_format();
        }

        /* Open/Close *********************************************/

        /*! Opens the given file and read header information. */
        bool open(string_type path, bool binary = false)
        {
					 /* if (binary)
								_stream.open(path.c_str(), std::ios::in | std::ios::binary);
						else*/
							_stream.open(path.c_str(), std::ios::in);

            if (!_stream.is_open()) {
                std::cerr << "Ply: can't open file \"" << path << "\""
                          << std::endl;
                return false;
            }

            string_type line_str;
            size_type line_num = 0;

            /* XXX: This will not work with Mac-style '\r' EOL */
            while (getline(_stream, line_str)) {
                /* The first line must be "ply" */
                if (line_num == 0) {
                    if (line_str != "ply") {
                        std::cerr << "Ply: invalid first line in header"
                                  << std::endl;
                        _stream.close();
                        return false;
                    } else {
                        line_num++;
                        continue;
                    }
                }

                /* The end of the header */
                if (line_str == "end_header") {
                    if (!check_end_of_header()) {
                        _stream.close();
                        return false;
                    } else
                        break;
                }

                /* Extract tokens */
                std::istringstream line_stream(line_str);
                string_type token;

                if (!(line_stream >> token)) {
                    std::cerr << "Ply: can't extract first token on line "
                              << (line_num + 1) << std::endl;
                    _stream.close();
                    return false;
                }

                if (token == "format") {
                    if (!parse_format(line_stream, line_num)) {
                        _stream.close();
                        return false;
                    }
                } else if (token == "comment") {
                    parse_comment(line_stream, line_num);
                } else if (token == "obj_info") {
                    parse_obj_info(line_stream, line_num);
                } else if (token == "element") {
                    if (!parse_element(line_stream, line_num)) {
                        _stream.close();
                        return false;
                    }
                } else if (token == "property") {
                    if (!parse_property(line_stream, line_num)) {
                        _stream.close();
                        return false;
                    }
                } else {
                    std::cerr << "Ply: unsupported token \"" << token
                              << "\" on line " << (line_num + 1) << std::endl;
                    _stream.close();
                    return false;
                }

                line_num++;
            }

			if (_format == PLY_FORMAT_BIN_BE || _format == PLY_FORMAT_BIN_LE)
{
	std::ios::streampos mark = _stream.tellg();
	_stream.close();
	_stream.open(path.c_str(), std::ios::in | std::ios::binary);
	_stream.seekg (mark, std::ios::beg);
}

            return true;
        }

        void close()
        {
            _stream.close();
        }

        /* Skip ***************************************************/

        /*! Skips the given element. */
        bool skip(const Element& element)
        {
            if (format() == PLY_FORMAT_ASC)
                return skip_ASC(element);
            else
                return skip_BIN(element);
        }

        /*! Skips the given property. */
        bool skip(const Property& property)
        {
            switch (format()) {
                case PLY_FORMAT_ASC:
                    return skip_ASC(property);
                    break;
                case PLY_FORMAT_BIN_BE:
                case PLY_FORMAT_BIN_LE:
                    return skip_BIN(property);
                    break;
                default: /* PLY_FORMAT_UNKNOWN */
                    assert(0);
                    return false;
                    break;
            }
        }

        /* Read ***************************************************/

        template<class T>
        bool read(const Property& property, T& value)
        {
            const Type type = property.type();
            return type_read(format(), native_format(),
                             type, _stream, value);
        }

        template<class T>
        bool read_count(const Property& property, T& count)
        {
            assert(property.type() == PLY_TYPE_LIST);
            const Type count_type = property.count_type();
            return type_read(format(), native_format(),
                             count_type, _stream, count);
        }

        template<class T>
        bool read_value(const Property& property, T& value)
        {
            assert(property.type() == PLY_TYPE_LIST);
            const Type value_type = property.value_type();
            return type_read(format(), native_format(),
                             value_type, _stream, value);
        }

        /* Format *************************************************/

        Format native_format() const
        {
            return _native_format;
        }

        Format format() const
        {
            return _format;
        }

        float version() const
        {
            return _version;
        }

        /* Comments ***********************************************/

        size_type num_comments() const
        {
            return _comments.size();
        }

        CommentsIterator comments_begin() const
        {
            return _comments.begin();
        }

        CommentsIterator comments_end() const
        {
            return _comments.end();
        }

        /* Object information *************************************/

        size_type num_obj_infos() const
        {
            return _obj_infos.size();
        }

        ObjInfosIterator obj_infos_begin() const
        {
            return _obj_infos.begin();
        }

        ObjInfosIterator obj_infos_end() const
        {
            return _obj_infos.end();
        }

        /* Elements ***********************************************/

        size_type num_elements() const
        {
            return _elements.size();
        }

        /*! Returns a for the elements. */
        ElementsIterator elements_begin() const
        {
            return _elements.begin();
        }

        /*! Returns a past-the-end iterator for the elements. */
        ElementsIterator elements_end() const
        {
            return _elements.end();
        }

        const Element& element(size_type i) const
        {
            return _elements[i];
        }

        /*! Finds an element with the given name. */
        ElementsIterator find_element(const string_type& name) const
        {
            ElementsIterator it;

            for (it = elements_begin(); it != elements_end(); ++it)
                if (it->name() == name)
                    return it;

            return it; /* = elements_end() */
        }

        /*! Checks whether an element with the given name exists. */
        bool has_element(const string_type& name) const
        {
            return (find_element(name) != elements_end());
        }

        /* Miscellaneous ******************************************/

        /*! Converts the given format string to the appropriate enum. */
        static Format format_string_to_enum(const string_type& s)
        {
            typedef std::pair<Format,string_type> FormatStringPair;

            static const FormatStringPair pairs[] = {
                FormatStringPair(PLY_FORMAT_ASC   , "ascii"               ),
                FormatStringPair(PLY_FORMAT_BIN_BE, "binary_big_endian"   ),
                FormatStringPair(PLY_FORMAT_BIN_LE, "binary_little_endian"),
            };
            static size_type num_pairs = sizeof(pairs)
                                       / sizeof(FormatStringPair);

            for (size_type i = 0; i != num_pairs; ++i)
                if (pairs[i].second == s)
                    return pairs[i].first;

            return PLY_FORMAT_UNKNOWN;
        }

        /*! Returns the size (in bytes) of the type or 0 if it a list. */
        static size_type type_size(Type type)
        {
            switch (type) {
                case PLY_TYPE_INT8: /* PLY_TYPE_CHAR */
                    return sizeof(int8_type);
                    break;
                case PLY_TYPE_UINT8: /* PLY_TYPE_UCHAR */
                    return sizeof(uint8_type);
                    break;
                case PLY_TYPE_INT16: /* PLY_TYPE_SHORT */
                    return sizeof(int16_type);
                    break;
                case PLY_TYPE_UINT16: /* PLY_TYPE_USHORT */
                    return sizeof(uint16_type);
                    break;
                case PLY_TYPE_INT32: /* PLY_TYPE_INT */
                    return sizeof(int32_type);
                    break;
                case PLY_TYPE_UINT32: /* PLY_TYPE_UINT */
                    return sizeof(uint32_type);
                    break;
                case PLY_TYPE_FLOAT32: /* PLY_TYPE_FLOAT */
                    return sizeof(float32_type);
                    break;
                case PLY_TYPE_FLOAT64: /* PLY_TYPE_DOUBLE */
                    return sizeof(float64_type);
                    break;
                default: /* PLY_TYPE_LIST, PLY_TYPE_UNKNOWN */
                    return 0;
                    break;
            }
        }

        /*! Converts the given type enum to its string representation. */
        static string_type type_enum_to_string(Type type)
        {
            switch (type) {
                case PLY_TYPE_CHAR: /* PLY_TYPE_INT8 */
                    return "char";
                    break;
                case PLY_TYPE_UCHAR: /* PLY_TYPE_UINT8 */
                    return "uchar";
                    break;
                case PLY_TYPE_SHORT: /* PLY_TYPE_INT16 */
                    return "short";
                    break;
                case PLY_TYPE_USHORT: /* PLY_TYPE_UINT16 */
                    return "ushort";
                    break;
                case PLY_TYPE_INT: /* PLY_TYPE_INT32 */
                    return "int";
                    break;
                case PLY_TYPE_UINT: /* PLY_TYPE_UINT32 */
                    return "uint";
                    break;
                case PLY_TYPE_FLOAT: /* PLY_TYPE_FLOAT32 */
                    return "float";
                    break;
                case PLY_TYPE_DOUBLE: /* PLY_TYPE_FLOAT64 */
                    return "double";
                    break;
                case PLY_TYPE_LIST:
                    return "list";
                    break;
                default: /* PLY_TYPE_UNKNOWN */
                    return "unknown";
                    break;
            }
        }

        /*! Converts the given type string to the appropriate enum. */
        static Type type_string_to_enum(const string_type& s)
        {
            typedef std::pair<Type,string_type> TypeStringPair;

            static const TypeStringPair pairs[] = {
                TypeStringPair(PLY_TYPE_CHAR   , "char"   ),
                TypeStringPair(PLY_TYPE_INT8   , "int8"   ),
                TypeStringPair(PLY_TYPE_UCHAR  , "uchar"  ),
                TypeStringPair(PLY_TYPE_UINT8  , "uint8"  ),
                TypeStringPair(PLY_TYPE_SHORT  , "short"  ),
                TypeStringPair(PLY_TYPE_INT16  , "int16"  ),
                TypeStringPair(PLY_TYPE_USHORT , "ushort" ),
                TypeStringPair(PLY_TYPE_UINT16 , "uint16" ),
                TypeStringPair(PLY_TYPE_INT    , "int"    ),
                TypeStringPair(PLY_TYPE_INT32  , "int32"  ),
                TypeStringPair(PLY_TYPE_UINT   , "uint"   ),
                TypeStringPair(PLY_TYPE_UINT32 , "uint32" ),
                TypeStringPair(PLY_TYPE_FLOAT  , "float"  ),
                TypeStringPair(PLY_TYPE_FLOAT32, "float32"),
                TypeStringPair(PLY_TYPE_DOUBLE , "double" ),
                TypeStringPair(PLY_TYPE_FLOAT64, "float64"),
                TypeStringPair(PLY_TYPE_LIST   , "list"   ),
            };
            static size_type num_pairs = sizeof(pairs)
                                       / sizeof(TypeStringPair);

            for (size_type i = 0; i != num_pairs; ++i)
                if (pairs[i].second == s)
                    return pairs[i].first;

            return PLY_TYPE_UNKNOWN;
        }

    protected:
        bool skip_ASC(const Element& element)
        {
            const size_type count = element.count();

            for (size_type i = 0; i != count; ++i)
                for (PropertiesIterator it = element.properties_begin();
                     it != element.properties_end(); ++it)
                    if (!skip_ASC(*it)) {
                        std::cerr << "Ply: error while skipping property \""
                                  << it->name() << "\" of element \""
                                  << element.name() << "\" " << (i + 1)
                                  << std::endl;
                        return false;
                    }

            return true;
        }

        bool skip_BIN(const Element& element)
        {
            const size_type count = element.count();
            const size_type size = element.size();

            if (size != 0)
                return (!_stream.seekg(count * size, std::ios::cur).good());

            for (size_type i = 0; i != count; ++i)
                for (PropertiesIterator it = element.properties_begin();
                     it != element.properties_end(); ++it)
                    if (!skip_BIN(*it)) {
                        std::cerr << "Ply: error while skipping property \""
                                  << it->name() << "\" of element \""
                                  << element.name() << "\" " << (i + 1)
                                  << std::endl;
                        return false;
                    }

            return true;
        }

        bool skip_ASC(const Property& property)
        {
            const Type type = property.type();

            assert(type != PLY_TYPE_UNKNOWN);

            if (type == PLY_TYPE_LIST) {
                const Type count_type = property.count_type();

                size_type count;

                if (!type_read_ASC(count_type, _stream, count))
                    return false;

                if (count == 0)
                    return true;

                const Type value_type = property.value_type();

                int dummy;

                for (size_type i = 0; i != count; ++i)
                    if (!type_read_ASC(value_type, _stream, dummy))
                        return false;
            } else {
                int dummy;

                if (!type_read_ASC(type, _stream, dummy))
                    return false;
            }

            return true;
        }

        bool skip_BIN(const Property& property)
        {
            const Type type = property.type();

            assert(type != PLY_TYPE_UNKNOWN);

            if (type == PLY_TYPE_LIST) {
                const Type count_type = property.count_type();
                const Type value_type = property.value_type();

                size_type count = 0;

                type_read(format(), native_format(),
                          count_type, _stream, count);

                const size_type value_type_size = type_size(value_type);

				return (!_stream.seekg(count * value_type_size, std::ios::cur).good());
            } else {
                const size_type size = type_size(type);

				return (!_stream.seekg(size, std::ios::cur).good());
            }
        }

        bool check_last_element() const
        {
            const Element& element = last_element();

            if (element.num_properties() == 0) {
                std::cerr << "Ply: element \"" << element.name()
                          << "\" has no property" << std::endl;
                return false;
            } else
                return true;
        }

        bool check_end_of_header() const
        {
            if (format() == PLY_FORMAT_UNKNOWN) {
                std::cerr << "Ply: unspecified format" << std::endl;
                return false;
            }

            if (num_elements() == 0) {
                std::cerr << "Ply: no element found" << std::endl;
                return false;
            }

            if (!check_last_element())
                return false;

            return true;
        }

        bool parse_format(std::istringstream& line_stream, size_type line_num)
        {
            if (line_num != 1) {
                std::cerr << "Ply: unexpected format token on line "
                          << (line_num + 1) << std::endl;
                return false;
            }

            string_type token;

            if (!(line_stream >> token)) {
                std::cerr << "Ply: can't extract format token on line "
                          << (line_num + 1) << std::endl;
                return false;
            }

            Format format = format_string_to_enum(token);

            if (format == PLY_FORMAT_UNKNOWN) {
                std::cerr << "Ply: unsupported format on line "
                          << (line_num + 1) << std::endl;
                return false;
            }

            if ((format == PLY_FORMAT_BIN_BE
              || format == PLY_FORMAT_BIN_LE)
             && native_format() == PLY_FORMAT_UNKNOWN) {
                std::cerr << "Ply: unknown native format" << std::endl;
                return false;
            }

            float version;

            if (!(line_stream >> version)) {
                std::cerr << "Ply: can't extract version token on line "
                          << (line_num + 1) << std::endl;
                return false;
            }

            if (line_stream >> token) {
                std::cerr << "Ply: unexpected token \"" << token
                          << "\" on line " << (line_num + 1) << std::endl;
                return false;
            }

            if (version != 1.0f) {
                std::cerr << "Ply: unsupported version on line "
                          << (line_num + 1) << std::endl;
                return false;
            }

            _format = format;
            _version = version;

            return true;
        }

        void parse_comment(std::istringstream& line_stream,
                           size_type line_num)
        {
            line_stream >> std::noskipws;
            char c;
            line_stream >> c; /* Skip first char */

            string_type comment;

            while (line_stream >> c)
                comment += c;

            _comments.push_back(comment);
        }

        void parse_obj_info(std::istringstream& line_stream,
                            size_type line_num)
        {
            line_stream >> std::noskipws;
            char c;
            line_stream >> c; /* Skip first char */

            string_type obj_info;

            while (line_stream >> c)
                obj_info += c;

            _obj_infos.push_back(obj_info);
        }

        bool parse_element(std::istringstream& line_stream,
                           size_type line_num)
        {
            if (num_elements() != 0)
                if (!check_last_element())
                    return false;

            string_type name;

            if (!(line_stream >> name)) {
                std::cerr << "Ply: can't extract name token on line "
                          << (line_num + 1) << std::endl;
                return false;
            }

            if (has_element(name)) {
                std::cerr << "Ply: already specified element on line "
                          << (line_num + 1) << std::endl;
                return false;
            }

            size_type count;

            if (!(line_stream >> count)) {
                std::cerr << "Ply: can't extract count token on line "
                          << (line_num + 1) << std::endl;
                return false;
            }

            string_type token;

            if (line_stream >> token) {
                std::cout << "Ply: unexpected token \"" << token
                          << "\" on line " << (line_num + 1) << std::endl;
                return false;
            }

            _elements.push_back(Element(name, count));

            return true;
        }

        bool parse_property(std::istringstream& line_stream,
                            size_type line_num)
        {
            if (num_elements() == 0) {
                std::cerr << "Ply: property specified w/o element on line "
                          << (line_num + 1) << std::endl;
                return false;
            }

            string_type token;

            if (!(line_stream >> token)) {
                std::cerr << "Ply: can't extract type token on line "
                          << (line_num + 1) << std::endl;
                return false;
            }

            Type type = type_string_to_enum(token);

            if (type == PLY_TYPE_UNKNOWN) {
                std::cerr << "Ply: unsupported type on line "
                          << (line_num + 1) << std::endl;
                return false;
            }

            Type count_type = PLY_TYPE_UNKNOWN;
            Type value_type = PLY_TYPE_UNKNOWN;

            if (type == PLY_TYPE_LIST) {
                if (!(line_stream >> token)) {
                    std::cerr << "Ply: can't extract count type token"
                                         " on line " << (line_num + 1)
                              << std::endl;
                    return false;
                }

                count_type = type_string_to_enum(token);

                if (count_type == PLY_TYPE_UNKNOWN) {
                    std::cerr << "Ply: unsupported count type on line "
                              << (line_num + 1) << std::endl;
                    return false;
                } else if (count_type == PLY_TYPE_CHAR
                        || count_type == PLY_TYPE_SHORT
                        || count_type == PLY_TYPE_INT
                        || count_type == PLY_TYPE_FLOAT
                        || count_type == PLY_TYPE_DOUBLE
                        || count_type == PLY_TYPE_LIST) {
                    std::cerr << "Ply: invalid count type ("
                              << type_enum_to_string(count_type)
                              << ") on line " << (line_num + 1) << std::endl;
                    return false;
                }

                if (!(line_stream >> token)) {
                    std::cerr << "Ply: can't extract value type token"
                                     " on line " << (line_num + 1)
                              << std::endl;
                    return false;
                }

                value_type = type_string_to_enum(token);

                if (value_type == PLY_TYPE_UNKNOWN) {
                    std::cerr << "Ply: unsupported value type on line "
                              << (line_num + 1) << std::endl;
                    return false;
                } else if (value_type == PLY_TYPE_LIST) {
                    std::cerr << "Ply: invalid value type (list) on line "
                              << (line_num + 1) << std::endl;
                    return false;
                }
            }

            if (!(line_stream >> token)) {
                std::cerr << "Ply: can't extract name token on line "
                          << (line_num + 1) << std::endl;
                return false;
            }

            /* Get the last element */
            Element& element = last_element();

            if (element.has_property(token)) {
                std::cerr << "Ply: element \"" << element.name()
                          << "\" already has a property \"" << token << '"' 
                          << (line_num + 1) << std::endl;
                return false;
            }

            bool b;

            if (type == PLY_TYPE_LIST)
                b = element.add_property(Property(token, count_type,
                                                         value_type));
            else
                b = element.add_property(Property(token, type));

            return b;
        }

        /*! Returns the last element (const version). */
        const Element& last_element() const
        {
            return *_elements.rbegin();
        }

        /*! Returns the last element (non-const version). */
        Element& last_element()
        {
            return *_elements.rbegin();
        }

        /*! Finds the byte order used by the machine. */
        void find_native_format()
        {
            const uint32_type u32 = 0x12345678;
            const uint8_type* ptr = reinterpret_cast<const uint8_type*>(&u32);

            if (ptr[0] == 0x12 && ptr[1] == 0x34
             && ptr[2] == 0x56 && ptr[3] == 0x78)
                _native_format = PLY_FORMAT_BIN_BE;
            else if (ptr[0] == 0x78 && ptr[1] == 0x56
                  && ptr[2] == 0x34 && ptr[3] == 0x12)
                _native_format = PLY_FORMAT_BIN_LE;
            else
                _native_format = PLY_FORMAT_UNKNOWN;
        }

        /*!
         *  Reads one item of the given type and converts it to the
         *  supplied type.
         */
        template<class T>
        static bool type_read(Format format, Format native_format, Type type,
                              std::istream& stream, T& t)
        {
            switch (format) {
                case PLY_FORMAT_ASC:
                    return type_read_ASC(type, stream, t);
                    break;
                case PLY_FORMAT_BIN_BE:
                    return type_read_BIN_BE(native_format, type, stream, t);
                    break;
                case PLY_FORMAT_BIN_LE:
                    return type_read_BIN_LE(native_format, type, stream, t);
                    break;
                default: /* PLY_FORMAT_UNKNOWN */
                    assert(0);
                    return false;
                    break;
            }
        }

//        /*!
//         *  Reads several items of the given type and converts them to the
//         *  supplied type.
//         */
//        template<class T>
//        static bool type_read(Format format, Format native_format, Type type,
//                              std::istream& stream, T* p, size_type n)
//        {
//            switch (format) {
//                case PLY_FORMAT_ASC:
//                    return type_read_ASC(type, p, n);
//                    break;
//                case PLY_FORMAT_BIN_BE:
//                    return type_read_BIN_BE(native_format, type, p, n);
//                    break;
//                case PLY_FORMAT_BIN_LE:
//                    return type_read_BIN_LE(native_format, type, p, n);
//                    break;
//                default: /* PLY_FORMAT_UNKNOWN */
//                    assert(0);
//                    return false;
//                    break;
//            }
//        }

        /* ASCII **************************************************/

        template<class T>
        static bool type_read_ASC(std::istream& stream, T& t)
        {
            if (!(stream >> t))
                return false;
            else
                return true;
        }

//        template<class T>
//        static bool type_read_ASC(std::istream& stream, T* p, size_type n)
//        {
//            for (size_type i = 0; i != n; ++i) {
//                T t;
//
//                if (!(stream >> t))
//                    return false;
//
//                *p++ = t;
//            }
//
//            return true;
//        }

        template<class T>
        static bool type_read_ASC(Type type, std::istream& stream, T& t);

//        template<class T>
//        static bool type_read_ASC(Type type,
//                                  std::istream& stream, T* p, size_type n);

        /* Binary *************************************************/

        /* Byte-swapping **********************/

        template<class T>
        static void swab(T& t)
        {
            const size_type size = sizeof(T);
            const size_type half_size = size / 2;

            uint8_type* p = reinterpret_cast<uint8_type*>(&t);

            for (size_type i = 0; i != half_size; ++i)
                std::swap(p[i], p[size - i - 1]);
        }

        template<class T>
        static void swab(T* p, size_type n)
        {
            const size_type size = sizeof(T);
            const size_type half_size = size / 2;

            uint8_type* q = reinterpret_cast<uint8_type*>(p);

            for (size_type i = 0; i != n; ++i)
                for (size_type j = 0; j != half_size; ++j)
                    std::swap(q[j], q[size - j - 1]);
        }

        /* Binary big-endian ******************/

        template<class T>
        static bool type_read_BIN_BE(Format native_format,
                                     std::istream& stream, T& t)
        {
            const size_type size = sizeof(T);

            char* p = reinterpret_cast<char*>(&t);

            if (!stream.read(p, size))
                return false;

            if (native_format != PLY_FORMAT_BIN_BE)
                swab(t);

            return true;
        }

//        template<class T>
//        static bool type_read_BIN_BE(Format native_format,
//                                     std::istream& stream, T* p, size_type n)
//        {
//            const size_type size = sizeof(T);
//
//            char* q = reinterpret_cast<char*>(p);
//
//            if (!stream.read(q, n * size))
//                return false;
//
//            if (native_format != PLY_FORMAT_BIN_BE)
//                swab(p, n);
//
//            return true;
//        }

        template<class T>
        static bool type_read_BIN_BE(Format native_format, Type type,
                                     std::istream& stream, T& t);

//        template<class T>
//        static bool type_read_BIN_BE(Format native_format, Type type,
//                                     std::istream& stream, T* p, size_type n);

        /* Binary little-endian ***************/

        template<class T>
        static bool type_read_BIN_LE(Format native_format,
                                     std::istream& stream, T& t)
        {
            const size_type size = sizeof(T);

            char* p = reinterpret_cast<char*>(&t);

            if (!stream.read(p, size))
                return false;

            if (native_format != PLY_FORMAT_BIN_LE)
                swab(t);

            return true;
        }

//        template<class T>
//        static bool type_read_BIN_LE(Format native_format,
//                                     std::istream& stream, T* p, size_type n)
//        {
//            const size_type size = sizeof(T);
//
//            char* q = reinterpret_cast<char*>(p);
//
//            if (!stream.read(q, n * size))
//                return false;
//
//            if (native_format != PLY_FORMAT_BIN_LE)
//                swab(p, n);
//
//            return true;
//        }

        template<class T>
        static bool type_read_BIN_LE(Format native_format, Type type,
                                     std::istream& stream, T& t);

//        template<class T>
//        static bool type_read_BIN_LE(Format native_format, Type type,
//                                     std::istream& stream, T* p, size_type n);

    protected:
        Format _format;
        Format _native_format;
        float _version;
        CommentList _comments;
        ObjInfoList _obj_infos;
        ElementList _elements;

        std::ifstream _stream;
};

template<>
inline bool Ply::type_read_ASC<Ply::int8_type>(std::istream& stream,
                                               int8_type& t)
{
    const int8_type min = std::numeric_limits<int8_type>::min();
    const int8_type max = std::numeric_limits<int8_type>::max();

    int i;

    if (!(stream >> i))
        return false;

    if (i < min || i > max)
        return false;

    t = int8_type(i);

    return false;
}

//template<>
//inline bool Ply::type_read_ASC<Ply::int8_type>(std::istream& stream,
//                                               int8_type* p, size_type n)
//{
//    const int8_type min = std::numeric_limits<int8_type>::min();
//    const int8_type max = std::numeric_limits<int8_type>::max();
//
//    for (size_type i = 0; i != n; ++i) {
//        int t;
//
//        if (!(stream >> t))
//            return false;
//
//        if (t < min || t > max)
//            return false;
//
//        *p++ = t;
//    }
//
//    return false;
//}

template<>
inline bool Ply::type_read_ASC<Ply::uint8_type>(std::istream& stream,
                                                uint8_type& t)
{
 // const uint8_type min = std::numeric_limits<uint8_type>::min();
    const uint8_type max = std::numeric_limits<uint8_type>::max();

    unsigned int u;

    if (!(stream >> u))
        return false;

    if (/* u < min || */ u > max)
        return false;

    t = uint8_type(u);

    return true;
}

//template<>
//inline bool Ply::type_read_ASC<Ply::uint8_type>(std::istream& stream,
//                                                uint8_type* p, size_type n)
//{
// // const uint8_type min = std::numeric_limits<uint8_type>::min();
//    const uint8_type max = std::numeric_limits<uint8_type>::max();
//
//    for (size_type i = 0; i != n; ++i) {
//        unsigned int t;
//
//        if (!(stream >> t))
//            return false;
//
//        if (/* t < min || */ t > max)
//            return false;
//
//        *p++ = t;
//    }
//
//    return true;
//}

template<class T>
bool Ply::type_read_ASC(Type type,
                        std::istream& stream, T& t)
{
    bool b;

    int8_type i8 = 0;
    uint8_type u8 = 0;
    int16_type i16 = 0;
    uint16_type u16 = 0;
    int32_type i32 = 0;
    uint32_type u32 = 0;
    float32_type f32 = 0;
    float64_type f64 = 0;

    switch (type) {
        case PLY_TYPE_INT8: /* PLY_TYPE_CHAR */
            b = type_read_ASC<int8_type>(stream, i8);
            t = T(i8);
            break;
        case PLY_TYPE_UINT8: /* PLY_TYPE_UCHAR */
            b = type_read_ASC<uint8_type>(stream, u8);
            t = T(u8);
            break;
        case PLY_TYPE_INT16: /* PLY_TYPE_SHORT */
            b = type_read_ASC<int16_type>(stream, i16);
            t = T(i16);
            break;
        case PLY_TYPE_UINT16: /* PLY_TYPE_USHORT */
            b = type_read_ASC<uint16_type>(stream, u16);
            t = T(u16);
            break;
        case PLY_TYPE_INT32: /* PLY_TYPE_INT */
            b = type_read_ASC<int32_type>(stream, i32);
            t = T(i32);
            break;
        case PLY_TYPE_UINT32: /* PLY_TYPE_UINT */
            b = type_read_ASC<uint32_type>(stream, u32);
            t = T(u32);
            break;
        case PLY_TYPE_FLOAT32: /* PLY_TYPE_FLOAT */
            b = type_read_ASC<float32_type>(stream, f32);
            t = T(f32);
            break;
        case PLY_TYPE_FLOAT64: /* PLY_TYPE_DOUBLE */
            b = type_read_ASC<float64_type>(stream, f64);
            t = T(f64);
            break;
        default: /* PLY_TYPE_LIST, PLY_TYPE_UNKNOWN */
            assert(0);
            return false;
            break;
    }

    return b;
}

template<class T>
bool Ply::type_read_BIN_BE(Format native_format, Type type,
                           std::istream& stream, T& t)
{
    bool b;

    int8_type i8 = 0;
    uint8_type u8 = 0;
    int16_type i16 = 0;
    uint16_type u16 = 0;
    int32_type i32 = 0;
    uint32_type u32 = 0;
    float32_type f32 = 0;
    float64_type f64 = 0;

    switch (type) {
        case PLY_TYPE_INT8: /* PLY_TYPE_CHAR */
            b = type_read_BIN_BE<int8_type>(native_format, stream, i8);
            t = T(i8);
            break;
        case PLY_TYPE_UINT8: /* PLY_TYPE_UCHAR */
            b = type_read_BIN_BE<uint8_type>(native_format, stream, u8);
            t = T(u8);
            break;
        case PLY_TYPE_INT16: /* PLY_TYPE_SHORT */
            b = type_read_BIN_BE<int16_type>(native_format, stream, i16);
            t = T(i16);
            break;
        case PLY_TYPE_UINT16: /* PLY_TYPE_USHORT */
            b = type_read_BIN_BE<uint16_type>(native_format, stream, u16);
            t = T(u16);
            break;
        case PLY_TYPE_INT32: /* PLY_TYPE_INT */
            b = type_read_BIN_BE<int32_type>(native_format, stream, i32);
            t = T(i32);
            break;
        case PLY_TYPE_UINT32: /* PLY_TYPE_UINT */
            b = type_read_BIN_BE<uint32_type>(native_format, stream, u32);
            t = T(u32);
            break;
        case PLY_TYPE_FLOAT32: /* PLY_TYPE_FLOAT */
            b = type_read_BIN_BE<float32_type>(native_format, stream, f32);
            t = T(f32);
            break;
        case PLY_TYPE_FLOAT64: /* PLY_TYPE_DOUBLE */
            b = type_read_BIN_BE<float64_type>(native_format, stream, f64);
            t = T(f64);
            break;
        default: /* PLY_TYPE_LIST, PLY_TYPE_UNKNOWN */
            assert(0);
            return false;
            break;
    }

    return b;
}

template<class T>
bool Ply::type_read_BIN_LE(Format native_format, Type type,
                           std::istream& stream, T& t)
{
    bool b;

    int8_type i8 = 0;
    uint8_type u8 = 0;
    int16_type i16 = 0;
    uint16_type u16 = 0;
    int32_type i32 = 0;
    uint32_type u32 = 0;
    float32_type f32 = 0;
    float64_type f64 = 0;

    switch (type) {
        case PLY_TYPE_INT8: /* PLY_TYPE_CHAR */
            b = type_read_BIN_LE<int8_type>(native_format, stream, i8);
            t = T(i8);
            break;
        case PLY_TYPE_UINT8: /* PLY_TYPE_UCHAR */
            b = type_read_BIN_LE<uint8_type>(native_format, stream, u8);
            t = T(u8);
            break;
        case PLY_TYPE_INT16: /* PLY_TYPE_SHORT */
            b = type_read_BIN_LE<int16_type>(native_format, stream, i16);
            t = T(i16);
            break;
        case PLY_TYPE_UINT16: /* PLY_TYPE_USHORT */
            b = type_read_BIN_LE<uint16_type>(native_format, stream, u16);
            t = T(u16);
            break;
        case PLY_TYPE_INT32: /* PLY_TYPE_INT */
            b = type_read_BIN_LE<int32_type>(native_format, stream, i32);
            t = T(i32);
            break;
        case PLY_TYPE_UINT32: /* PLY_TYPE_UINT */
            b = type_read_BIN_LE<uint32_type>(native_format, stream, u32);
            t = T(u32);
            break;
        case PLY_TYPE_FLOAT32: /* PLY_TYPE_FLOAT */
            b = type_read_BIN_LE<float32_type>(native_format, stream, f32);
            t = T(f32);
            break;
        case PLY_TYPE_FLOAT64: /* PLY_TYPE_DOUBLE */
            b = type_read_BIN_LE<float64_type>(native_format, stream, f64);
            t = T(f64);
            break;
        default: /* PLY_TYPE_LIST, PLY_TYPE_UNKNOWN */
            assert(0);
            return false;
            break;
    }

    return b;
}

} /* misc */

} /* I */

#endif /* I_MISC_PLY_HPP */
