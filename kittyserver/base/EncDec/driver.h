// DRIVER.CC - test driver for the C++/object oriented translation and 
//             modification of MD5.

// Translation and modification (c) 1995 by Mordechai T. Abzug 

// This translation/ modification is provided "as is," without express or 
// implied warranty of any kind.

// The translator/ modifier does not claim (1) that MD5 will do what you think 
// it does; (2) that this translation/ modification is accurate; or (3) that 
// this software is "merchantible."  (Language for this disclaimer partially 
// copied from the disclaimer below).

/* based on:

   MDDRIVER.C - test driver for MD2, MD4 and MD5

  Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
rights reserved.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>

#include "md5.h"


// Length of test block, number of test blocks.

#define TEST_BLOCK_LEN 1000
#define TEST_BLOCK_COUNT 1000

static void  MD5_string (unsigned char *string , std::string& md5);

// Digests a string and prints the result.

void MD5_string (unsigned char *string , std::string& md5){

  MD5 context;
  unsigned int len = strlen ( (char *)string);

  context.update   (string, len);
  context.finalize ();
  char *tmp = context.hex_digest();
  if(tmp != NULL)
  {
	  md5 = tmp;
	  delete[] tmp;
	  tmp = NULL;
  }
}
