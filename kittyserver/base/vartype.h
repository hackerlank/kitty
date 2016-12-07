/*
                 __________________        __________          ___________________
                /                 /\      /         /\        /                  /\
               /   ______________/ /     /___   ___/ /       /  _____________   / /
              /   / \____________\/      \__/  / __\/       /  /\__________ /  / /
             /   / /                       /  / /          /  / /          /  / /
            /   /_/____________           /  / /          /  /_/__________/  / /
           /                  /\         /  / /          /                  / /
          /  ________________/ /        /  / /          /      ____________/ /
         /  / _______________\/        /  / /          /  /\   \___________\/
        /  / /                        /  / /          /  / /\   \
       /  / /                        /  / /          /  / /  \   \
      /  / /                        /  / /          /  / /    \   \
     /  / /                     ___/  /_/_         /  / /      \   \
    /  / /                     /         /\       /  / /        \   \
   /__/ /                     /_________/ /      /__/ /          \___\
   \__\/                      \_________\/       \__\/            \___\


	Fir Game Engine

	@file vartype.h
	@$Id: vartype.h 268 2013-06-04 06:45:12Z  $
	@brief	vartype, 动态数据类型变量
	@author My name is CHENG XIE, and I am your God, wa hahaha...
	@date 2013-04-25

*/
#ifndef __VARTYPE_H__
#define __VARTYPE_H__
#include "Fir.h"
#include "zString.h"

namespace Fir {

	class VarType {
		public:
			VarType() {
				clear();
			}

			virtual ~VarType() {}

			VarType(const VarType &val) {
				clear();
				_var_string = val._var_string;
			}

			VarType(const char *val) {
				clear();
				_var_string = val ? val : "";
			}

			VarType(const std::string &val) {
				clear();
				_var_string = val;
			}

			VarType(unsigned long val) {
				char tmp[256];
				snprintf(tmp, 255, "%lu", val);
				clear();
				_var_string = tmp;
			}
			
			VarType(unsigned int val) {
				char tmp[256];
				snprintf(tmp, 255, "%u", val);
				clear();
				_var_string = tmp;
			}

			VarType(unsigned short val) {
				char tmp[256];
				snprintf(tmp, 255, "%u", val);
				clear();
				_var_string = tmp;
			}

			VarType(byte val) {
				char tmp[256];
				snprintf(tmp, 255, "%u", val);
				clear();
				_var_string = tmp;
			}

			VarType(long val) {
				char tmp[256];
				snprintf(tmp, 255, "%ld", val);
				clear();
				_var_string = tmp;
			}

			VarType(int val) {
				char tmp[256];
				snprintf(tmp, 255, "%d", val);
				clear();
				_var_string = tmp;
			}

			VarType(short val) {
				char tmp[256];
				snprintf(tmp, 255, "%d", val);
				clear();
				_var_string = tmp;
			}

			VarType(char val) {
				char tmp[256];
				snprintf(tmp, 255, "%d", val);
				clear();
				_var_string = tmp;
			}

			VarType(float val) {
				char tmp[256];
				snprintf(tmp, 255, "%f", val);
				clear();
				_var_string = tmp;
			}

			VarType(double val) {
				char tmp[256];
				snprintf(tmp, 255, "%f", val);
				clear();
				_var_string = tmp;
			}

			
			VarType(bool val) {
				clear();
				_var_string = val ? "true" : "false";
			}

			VarType(const std::vector<VarType> &val) {
				clear();
				for (size_t i = 0; i < val.size(); ++i) {
					_var_string += (const char *)val[i];
					if (i < val.size() - 1)
						_var_string += ":";	
				}
			}
        
			VarType(const std::map<VarType, VarType> &val) {
				clear();
				size_t i = 0;
				for (std::map<VarType, VarType>::const_iterator it = val.begin(); it != val.end(); ++it, ++i) {
					_var_string += (const char *)it->first;
					_var_string += "|";
					_var_string += (const char *)it->second;
					if (i < val.size() - 1)
						_var_string += ":";	
				}
			}

            bool operator == (const VarType &val) const {
                return 0 == strcmp(_var_string.c_str(), val._var_string.c_str());
            }
        
            bool operator == (const char *val) const {
                return 0 == strcmp(_var_string.c_str(), val);
            }
        
            bool operator == (const std::string &val) const {
                return _var_string == val;
            }

			bool operator == (unsigned long val) const {
				if (!_var_ulong) 
					_var_ulong = (unsigned long)atol(_var_string.c_str());
			    return _var_ulong == val;
			}
			
			bool operator == (unsigned int val) const {
				if (!_var_uint) 
					_var_uint = (unsigned int)atoi(_var_string.c_str());
                return _var_uint == val;
			}

			bool operator == (unsigned short val) const {
				if (!_var_ushort)
					_var_ushort = (unsigned short)atoi(_var_string.c_str());
                return _var_ushort == val;
			}

			bool operator == (byte val) const {
				if (!_var_byte)
					_var_byte = (byte)atoi(_var_string.c_str());
                return _var_byte == val;
			}

			bool operator == (long val) const {
				if (!_var_long)
					_var_long = atol(_var_string.c_str());
                return _var_long == val;
			}

			bool operator == (int val) const {
				if (!_var_int)
					_var_int = atoi(_var_string.c_str());
                return _var_int == val;
			}

			bool operator == (short val) const {
				if (!_var_short)
					_var_short = (short)atoi(_var_string.c_str());
                return _var_short == val;
			}

			bool operator == (char val) const {
				if (!_var_char)
					_var_char = (char)atoi(_var_string.c_str());
                return _var_char == val;
			}

			bool operator == (float val) const {
				if (!_var_float)
					_var_float = (float)atof(_var_string.c_str());
                return _var_float == val;
			}

			bool operator == (double val) const {
				if (!_var_double)
					_var_double = atof(_var_string.c_str());
				return _var_double == val;
			}

			bool operator == (bool val) const {
				if (!_var_bool)
					_var_bool = _var_string == "true";
                return _var_bool == val;
			}

			bool operator == (const std::vector<VarType> &val) const {
				std::string tmp;
				for (size_t i = 0; i < val.size(); ++i) {
					tmp += (const char *)val[i];
					if (i < val.size() - 1)
						tmp += ":";	
				}
				return tmp == _var_string;
			}
			
			bool operator == (const std::map<VarType, VarType> &val) const {
				std::string tmp;
				size_t i = 0;
				for (std::map<VarType, VarType>::const_iterator it = val.begin(); it != val.end(); ++it, ++i) {
					tmp += (const char *)it->first;
					tmp += "|";
					tmp += (const char *)it->second;
					if (i < val.size() - 1)
						tmp += ":";	
				}
				return tmp == _var_string;
			}
        
            bool operator < (const VarType &val) const {
                bool isstring = false;
                for (size_t i = 0; i < _var_string.size(); ++i) {
                    if (!isdigit(_var_string[i])) {
                        isstring = true;
                        break;
                    }
                }
                if (!isstring) {
                    for (size_t i = 0; i < val._var_string.size(); ++i) {
                        if (!isdigit(val._var_string[i])) {
                            isstring = true;
                            break;
                        }
                    }
                }
                if (!isstring) {
                    _var_ulong ? _var_ulong : _var_ulong = (unsigned long)atol(_var_string.c_str());
                    return _var_ulong < (unsigned long)val;
                }
        
                return 0 != strcmp(_var_string.c_str(), val._var_string.c_str());
            }

			const VarType & operator = (const VarType &val) {
				clear();
				_var_string = val._var_string;
				return *this;
			}	
			
			const VarType & operator = (const char *val) {
				clear();
				_var_string = val;
				return *this;
			}

			const VarType & operator = (const std::string &val) {
				clear();
				_var_string = val;
				return *this;
			}

			const VarType & operator = (unsigned long val) {
				char tmp[256];
				snprintf(tmp, 255, "%lu", val);
				clear();
				_var_string = tmp;
				return *this;
			}
			
			const VarType & operator = (unsigned int val) {
				char tmp[256];
				snprintf(tmp, 255, "%u", val);
				clear();
				_var_string = tmp;
				return *this;
			}

			const VarType & operator = (unsigned short val) {
				char tmp[256];
				snprintf(tmp, 255, "%u", val);
				clear();
				_var_string = tmp;
				return *this;
			}

			const VarType & operator = (byte val) {
				char tmp[256];
				snprintf(tmp, 255, "%u", val);
				clear();
				_var_string = tmp;
				return *this;
			}

			const VarType & operator = (long val) {
				char tmp[256];
				snprintf(tmp, 255, "%ld", val);
				clear();
				_var_string = tmp;
				return *this;
			}

			const VarType & operator = (int val) {
				char tmp[256];
				snprintf(tmp, 255, "%d", val);
				clear();
				_var_string = tmp;
				return *this;
			}

			const VarType & operator = (short val) {
				char tmp[256];
				snprintf(tmp, 255, "%d", val);
				clear();
				_var_string = tmp;
				return *this;
			}

			const VarType & operator = (char val) {
				char tmp[256];
				snprintf(tmp, 255, "%d", val);
				clear();
				_var_string = tmp;
				return *this;
			}

			const VarType & operator = (float val) {
				char tmp[256];
				snprintf(tmp, 255, "%f", val);
				clear();
				_var_string = tmp;
				return *this;
			}

			const VarType & operator = (double val) {
				char tmp[256];
				snprintf(tmp, 255, "%f", val);
				clear();
				_var_string = tmp;
				return *this;
			}
			
			
			const VarType & operator = (bool val) {
				clear();
				_var_string = val ? "true" : "false";
				return *this;
			}

			const VarType & operator = (const std::vector<VarType> &val) {
				clear();
				for (size_t i = 0; i < val.size(); ++i) {
					_var_string += (const char *)val[i];
					if (i < val.size() - 1)
						_var_string += ":";	
				}
				return *this;
			}

			const VarType & operator = (const std::map<VarType, VarType> &val) {
				clear();
				size_t i = 0;
				for (std::map<VarType, VarType>::const_iterator it = val.begin(); it != val.end(); ++it, ++i) {
					_var_string += (const char *)it->first;
					_var_string += "|";
					_var_string += (const char *)it->second;
					if (i < val.size() - 1)
						_var_string += ":";	
				}
				return *this;
			}

			operator const char * () const {
				return _var_string.c_str();
			}

			operator const std::string & () const {
				return _var_string;
			}
        
			operator unsigned long () const {	
				return _var_ulong ? _var_ulong : _var_ulong = (unsigned long)atol(_var_string.c_str());
			}
			
			operator unsigned int () const {
				return _var_uint ? _var_uint : _var_uint = (unsigned int)atoi(_var_string.c_str());
			}

			operator unsigned short () const {
				return _var_ushort ? _var_ushort : _var_ushort = (unsigned short)atoi(_var_string.c_str());
			}

			operator byte () const {
				return _var_byte ? _var_byte : _var_byte = (byte)atoi(_var_string.c_str());
			}

			operator long () const {
				return _var_long ? _var_long : _var_long = atol(_var_string.c_str());
			}

			operator int () const {
				return _var_int ? _var_int : _var_int = atoi(_var_string.c_str());
			}

			operator short () const {
				return _var_short ? _var_short : _var_short = (short)atoi(_var_string.c_str());
			}

			operator char () const {
				return _var_char ? _var_char : _var_char = (char)atoi(_var_string.c_str());
			}

			operator float () const {
				return _var_float ? _var_float : _var_float = (float)atof(_var_string.c_str());
			}

			operator double () const {
				return _var_double ? _var_double : _var_double = atof(_var_string.c_str());
			}

			time_t time() const {
				if (_var_timet) return _var_timet;

				tm tv;
				time_t theTime = 0;

				if (atol(_var_string.c_str()) != 0)
					_var_timet = atol(_var_string.c_str());
				else
				{
					if (NULL == strptime(_var_string.c_str(), "%Y%m%d %H:%M:%S", &tv))
						_var_timet = 0;
					else
					{
						theTime = timegm(&tv);
						if ((time_t)-1 == theTime)
							_var_timet = 0;
						else
							_var_timet = theTime - 8*60*60;//UTC时间
					}
				}

				return _var_timet;
			}

			operator bool () const {
				return _var_bool ? _var_bool : _var_bool = _var_string == "true";
			}

			operator std::vector<VarType> & () const {
				if (_var_vector.empty()) 
					stringtok< std::vector<VarType> >(_var_vector, _var_string, ":");
				return _var_vector;
			}

			operator std::map<VarType, VarType> & () const {
				if (_var_map.empty()) {
					if (_var_vector.empty()) 
						stringtok< std::vector<VarType> >(_var_vector, _var_string, ":");
					for (size_t i = 0; i < _var_vector.size(); ++i) {
						std::vector<VarType> tmp;
						stringtok< std::vector<VarType> >(tmp, _var_vector[i], "|");
						if (tmp.size() == 2)
							_var_map[tmp[0]] = tmp[1];
					}
				}
				return _var_map;
			}

		private:

			void clear() {
				_var_string = "";
			    _var_ulong = 0;
                _var_uint = 0;
                _var_ushort = 0;
                _var_byte = 0;
                _var_long = 0;
                _var_int = 0;
                _var_short = 0;
                _var_char = 0;
                _var_float = 0;
                _var_double = 0;
				_var_timet = 0;
				_var_bool = false;
				_var_vector.clear();
				_var_map.clear();
			}

		private:
			
			std::string				_var_string;	
			mutable unsigned long 	_var_ulong;
			mutable unsigned int 	_var_uint;
			mutable unsigned short 	_var_ushort;
			mutable byte 			_var_byte;
			mutable long 			_var_long;
			mutable int 			_var_int;
			mutable short 			_var_short;
			mutable char 			_var_char;
			mutable float 			_var_float;
			mutable double 			_var_double;
			mutable time_t 			_var_timet;
			mutable bool			_var_bool;
			mutable std::vector<VarType> _var_vector;
			mutable std::map<VarType, VarType> _var_map;
	};
}

#endif
