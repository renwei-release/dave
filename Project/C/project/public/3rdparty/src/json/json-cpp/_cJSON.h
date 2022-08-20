/*
 Copyright (c) 2009 Dave Gamble

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#ifndef _cJSON__h
#define _cJSON__h

#include <stdint.h>

typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;


#ifdef __cplusplus
extern "C"
{
#endif

/* _cJSON Types: */
#define _cJSON_False 0
#define _cJSON_True 1
#define _cJSON_NULL 2
#define _cJSON_Int 3
#define _cJSON_Double 4
#define _cJSON_String 5
#define _cJSON_Array 6
#define _cJSON_Object 7

#define _cJSON_IsReference 256

/* The _cJSON structure: */
typedef struct _cJSON
{
    struct _cJSON *next, *prev; /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct _cJSON *child; /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

    int type; /* The type of the item, as above. */

    char *valuestring; /* The item's string, if type==_cJSON_String */
    int64 valueint; /* The item's number, if type==_cJSON_Number */
    double valuedouble; /* The item's number, if type==_cJSON_Number */
    int sign;   /* sign of valueint, 1(unsigned), -1(signed) */

    char *string; /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} _cJSON;

typedef struct _cJSON_Hooks
{
    void *(*malloc_fn)(size_t sz);
    void (*free_fn)(void *ptr);
} _cJSON_Hooks;

/* Supply malloc, realloc and free functions to _cJSON */
extern void _cJSON_InitHooks(_cJSON_Hooks* hooks);

/* Supply a block of JSON, and this returns a _cJSON object you can interrogate. Call _cJSON_Delete when finished. */
extern _cJSON *_cJSON_Parse(const char *value, const char **ep);
/* Render a _cJSON entity to text for transfer/storage. Free the char* when finished. */
extern char *_cJSON_Print(_cJSON *item);
/* Render a _cJSON entity to text for transfer/storage without any formatting. Free the char* when finished. */
extern char *_cJSON_PrintUnformatted(_cJSON *item);
/* Delete a _cJSON entity and all subentities. */
extern void _cJSON_Delete(_cJSON *c);

/* Returns the number of items in an array (or object). */
extern int _cJSON_GetArraySize(_cJSON *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
extern _cJSON *_cJSON_GetArrayItem(_cJSON *array, int item);
/* Get item "string" from object. Case insensitive. */
extern _cJSON *_cJSON_GetObjectItem(_cJSON *object, const char *string);

/* remove gloal variable for thread safe. --by Bwar on 2020-11-15 */
/* for analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when _cJSON_Parse() returns 0. 0 when _cJSON_Parse() succeeds. */
/* extern const char *_cJSON_GetErrorPtr(); */

/* These calls create a _cJSON item of the appropriate type. */
extern _cJSON *_cJSON_CreateNull();
extern _cJSON *_cJSON_CreateTrue();
extern _cJSON *_cJSON_CreateFalse();
extern _cJSON *_cJSON_CreateBool(int b);
extern _cJSON *_cJSON_CreateDouble(double num, int sign);
extern _cJSON *_cJSON_CreateInt(uint64 num, int sign);
extern _cJSON *_cJSON_CreateString(const char *string);
extern _cJSON *_cJSON_CreateArray();
extern _cJSON *_cJSON_CreateObject();

/* These utilities create an Array of count items. */
extern _cJSON *_cJSON_CreateIntArray(int *numbers, int sign, int count);
extern _cJSON *_cJSON_CreateFloatArray(float *numbers, int count);
extern _cJSON *_cJSON_CreateDoubleArray(double *numbers, int count);
extern _cJSON *_cJSON_CreateStringArray(const char **strings, int count);

/* Append item to the specified array/object. */
extern void _cJSON_AddItemToArray(_cJSON *array, _cJSON *item);
extern void _cJSON_AddItemToArrayHead(_cJSON *array, _cJSON *item);    /* add by Bwar on 2015-01-28 */
extern void _cJSON_AddItemToObject(_cJSON *object, const char *string,
                _cJSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing _cJSON to a new _cJSON, but don't want to corrupt your existing _cJSON. */
extern void _cJSON_AddItemReferenceToArray(_cJSON *array, _cJSON *item);
extern void _cJSON_AddItemReferenceToObject(_cJSON *object, const char *string,
                _cJSON *item);

/* Remove/Detatch items from Arrays/Objects. */
extern _cJSON *_cJSON_DetachItemFromArray(_cJSON *array, int which);
extern void _cJSON_DeleteItemFromArray(_cJSON *array, int which);
extern _cJSON *_cJSON_DetachItemFromObject(_cJSON *object, const char *string);
extern void _cJSON_DeleteItemFromObject(_cJSON *object, const char *string);

/* Update array items. */
extern void _cJSON_ReplaceItemInArray(_cJSON *array, int which, _cJSON *newitem);
extern void _cJSON_ReplaceItemInObject(_cJSON *object, const char *string,
                _cJSON *newitem);

#define _cJSON_AddNullToObject(object,name)	_cJSON_AddItemToObject(object, name, _cJSON_CreateNull())
#define _cJSON_AddTrueToObject(object,name)	_cJSON_AddItemToObject(object, name, _cJSON_CreateTrue())
#define _cJSON_AddFalseToObject(object,name)		_cJSON_AddItemToObject(object, name, _cJSON_CreateFalse())
#define _cJSON_AddNumberToObject(object,name,n)	_cJSON_AddItemToObject(object, name, _cJSON_CreateNumber(n))
#define _cJSON_AddStringToObject(object,name,s)	_cJSON_AddItemToObject(object, name, _cJSON_CreateString(s))


#ifdef __cplusplus
}
#endif

#endif
