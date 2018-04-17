/* * U:Kit ESP8266 Firmware - This is the 'smart' firmware for the U:Kit sensor kit
 * Copyright (C) 2016, 2018 Slavey Karadzhov <slav@attachix.com>
 *
 * This file is part of U:Kit ESP8266 Firmware.
 *
 * U:Kit ESP8266 Firmware is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * U:Kit ESP8266 Firmware is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with U:Kit ESP8266 Firmware.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "jsvm-ext.h"
#include "m_printf.h"

extern void uart_tx_one_char(char ch);

#define jerryx_port_handler_print_char(A) uart_tx_one_char(A)

jerry_value_t alertFunction (const jerry_value_t function_obj,
							 const jerry_value_t this_val,
							 const jerry_value_t args_p[],
							 const jerry_length_t args_cnt)
{
	  jerry_value_t str_val = jerry_value_to_string (args_p[0]);

	  jerry_size_t req_sz = jerry_get_string_size (str_val);
	  jerry_char_t str_buf_p[req_sz + 1];

	  jerry_string_to_char_buffer (str_val, str_buf_p, req_sz);
	  str_buf_p[req_sz] = '\0';

	  m_printf("%s",(char *)str_buf_p);
	  jerry_release_value(str_val);

	  return jerry_create_boolean (true);
}

jerry_value_t printFunction (const jerry_value_t function_obj,
							 const jerry_value_t this_val,
							 const jerry_value_t args_p[],
							 const jerry_length_t args_cnt)
{
	  (void) function_obj; /* unused */
	  (void) this_val; /* unused */

	  static const char *null_str = "\\u0000";
	  jerry_length_t arg_index = 0;
	  jerry_size_t chr_index = 0;
	  jerry_size_t null_index = 0;

	  jerry_value_t ret_val = jerry_create_undefined ();

	  for (arg_index = 0;
	       jerry_value_is_undefined (ret_val) && arg_index < args_cnt;
	       arg_index++)
	  {
	    jerry_value_t str_val = jerry_value_to_string (args_p[arg_index]);

	    if (!jerry_value_has_error_flag (str_val))
	    {
	      if (arg_index != 0)
	      {
	        jerryx_port_handler_print_char (' ');
	      }

	      jerry_size_t substr_size;
	      jerry_length_t substr_pos = 0;
	      jerry_char_t substr_buf[256];

	      while ((substr_size = jerry_substring_to_char_buffer (str_val,
	                                                            substr_pos,
	                                                            substr_pos + 256,
	                                                            substr_buf,
	                                                            256)) != 0)
	      {
	        for (chr_index = 0; chr_index < substr_size; chr_index++)
	        {
	          char chr = (char) substr_buf[chr_index];
	          if (chr == '\0')
	          {
	            for (null_index = 0; null_str[null_index] != 0; null_index++)
	            {
	              jerryx_port_handler_print_char (null_str[null_index]);
	            }
	          }
	          else
	          {
	            jerryx_port_handler_print_char (chr);
	          }
	        }

	        substr_pos += substr_size;
	      }

	      jerry_release_value (str_val);
	    }
	    else
	    {
	      ret_val = str_val;
	    }
	  }

	  jerryx_port_handler_print_char ('\n');

	  return ret_val;
}
