#include "SWIG_header.inl"

/* -------- TYPES TABLE (BEGIN) -------- */

#define SWIGTYPE_p_ALLEGRO_BITMAP swig_types[0]
#define SWIGTYPE_p_ALLEGRO_COLOR swig_types[1]
#define SWIGTYPE_p_ALLEGRO_INDEX_BUFFER swig_types[2]
#define SWIGTYPE_p_ALLEGRO_LINE_CAP swig_types[3]
#define SWIGTYPE_p_ALLEGRO_LINE_JOIN swig_types[4]
#define SWIGTYPE_p_ALLEGRO_PRIM_ATTR swig_types[5]
#define SWIGTYPE_p_ALLEGRO_PRIM_BUFFER_FLAGS swig_types[6]
#define SWIGTYPE_p_ALLEGRO_PRIM_STORAGE swig_types[7]
#define SWIGTYPE_p_ALLEGRO_PRIM_TYPE swig_types[8]
#define SWIGTYPE_p_ALLEGRO_VERTEX swig_types[9]
#define SWIGTYPE_p_ALLEGRO_VERTEX_BUFFER swig_types[10]
#define SWIGTYPE_p_ALLEGRO_VERTEX_DECL swig_types[11]
#define SWIGTYPE_p_ALLEGRO_VERTEX_ELEMENT swig_types[12]
#define SWIGTYPE_p_float swig_types[13]
#define SWIGTYPE_p_int swig_types[14]
#define SWIGTYPE_p_void swig_types[15]
static swig_type_info *swig_types[17];
static swig_module_info swig_module = {swig_types, 16, 0, 0, 0, 0};
#define SWIG_TypeQuery(name) SWIG_TypeQueryModule(&swig_module, &swig_module, name)
#define SWIG_MangledTypeQuery(name) SWIG_MangledTypeQueryModule(&swig_module, &swig_module, name)

/* -------- TYPES TABLE (END) -------- */

#define SWIG_name      "lallegro_primitives"
#define SWIG_init      luaopen_lallegro_primitives
#define SWIG_init_user luaopen_lallegro_primitives_user

#define SWIG_LUACODE   luaopen_lallegro_primitives_luacode

#include <allegro5/allegro.h>
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"


#ifdef __cplusplus	/* generic alloc/dealloc fns*/
#define SWIG_ALLOC_ARRAY(TYPE,LEN) 	new TYPE[LEN]
#define SWIG_FREE_ARRAY(PTR)		delete[] PTR
#else
#define SWIG_ALLOC_ARRAY(TYPE,LEN) 	(TYPE *)malloc(LEN*sizeof(TYPE))
#define SWIG_FREE_ARRAY(PTR)		free(PTR)
#endif
/* counting the size of arrays:*/
SWIGINTERN int SWIG_itable_size(lua_State* L, int index)
{
	int n=0;
	while(1){
		lua_rawgeti(L,index,n+1);
		if (lua_isnil(L,-1))break;
		++n;
		lua_pop(L,1);
	}
	lua_pop(L,1);
	return n;
}

SWIGINTERN int SWIG_table_size(lua_State* L, int index)
{
	int n=0;
	lua_pushnil(L);  /* first key*/
	while (lua_next(L, index) != 0) {
		++n;
		lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration*/
	}
	return n;
}

/* super macro to declare array typemap helper fns */
#define SWIG_DECLARE_TYPEMAP_ARR_FN(NAME,TYPE)\
	SWIGINTERN int SWIG_read_##NAME##_num_array(lua_State* L,int index,TYPE *array,int size){\
		int i;\
		for (i = 0; i < size; i++) {\
			lua_rawgeti(L,index,i+1);\
			if (lua_isnumber(L,-1)){\
				array[i] = (TYPE)lua_tonumber(L,-1);\
			} else {\
				lua_pop(L,1);\
				return 0;\
			}\
			lua_pop(L,1);\
		}\
		return 1;\
	}\
	SWIGINTERN TYPE* SWIG_get_##NAME##_num_array_fixed(lua_State* L, int index, int size){\
		TYPE *array;\
		if (!lua_istable(L,index) || SWIG_itable_size(L,index) != size) {\
			SWIG_Lua_pushferrstring(L,"expected a table of size %d",size);\
			return 0;\
		}\
		array=SWIG_ALLOC_ARRAY(TYPE,size);\
		if (!SWIG_read_##NAME##_num_array(L,index,array,size)){\
			SWIG_Lua_pusherrstring(L,"table must contain numbers");\
			SWIG_FREE_ARRAY(array);\
			return 0;\
		}\
		return array;\
	}\
	SWIGINTERN TYPE* SWIG_get_##NAME##_num_array_var(lua_State* L, int index, int* size)\
	{\
		TYPE *array;\
		if (!lua_istable(L,index)) {\
			SWIG_Lua_pusherrstring(L,"expected a table");\
			return 0;\
		}\
		*size=SWIG_itable_size(L,index);\
		if (*size<1){\
			SWIG_Lua_pusherrstring(L,"table appears to be empty");\
			return 0;\
		}\
		array=SWIG_ALLOC_ARRAY(TYPE,*size);\
		if (!SWIG_read_##NAME##_num_array(L,index,array,*size)){\
			SWIG_Lua_pusherrstring(L,"table must contain numbers");\
			SWIG_FREE_ARRAY(array);\
			return 0;\
		}\
		return array;\
	}\
	SWIGINTERN void SWIG_write_##NAME##_num_array(lua_State* L,TYPE *array,int size){\
		int i;\
		lua_newtable(L);\
		for (i = 0; i < size; i++){\
			lua_pushnumber(L,(lua_Number)array[i]);\
			lua_rawseti(L,-2,i+1);/* -1 is the number, -2 is the table*/ \
		}\
	}

SWIG_DECLARE_TYPEMAP_ARR_FN(schar,signed char)
SWIG_DECLARE_TYPEMAP_ARR_FN(uchar,unsigned char)
SWIG_DECLARE_TYPEMAP_ARR_FN(int,int)
SWIG_DECLARE_TYPEMAP_ARR_FN(uint,unsigned int)
SWIG_DECLARE_TYPEMAP_ARR_FN(short,short)
SWIG_DECLARE_TYPEMAP_ARR_FN(ushort,unsigned short)
SWIG_DECLARE_TYPEMAP_ARR_FN(long,long)
SWIG_DECLARE_TYPEMAP_ARR_FN(ulong,unsigned long)
SWIG_DECLARE_TYPEMAP_ARR_FN(float,float)
SWIG_DECLARE_TYPEMAP_ARR_FN(double,double)

SWIGINTERN int SWIG_read_ptr_array(lua_State* L,int index,void **array,int size,swig_type_info *type){
	int i;
	for (i = 0; i < size; i++) {
		lua_rawgeti(L,index,i+1);
		if (!lua_isuserdata(L,-1) || SWIG_ConvertPtr(L,-1,&array[i],type,0)==-1){
			lua_pop(L,1);
			return 0;
		}
		lua_pop(L,1);
	}
	return 1;
}
SWIGINTERN void** SWIG_get_ptr_array_fixed(lua_State* L, int index, int size,swig_type_info *type){
	void **array;
	if (!lua_istable(L,index) || SWIG_itable_size(L,index) != size) {
		SWIG_Lua_pushferrstring(L,"expected a table of size %d",size);
		return 0;
	}
	array=SWIG_ALLOC_ARRAY(void*,size);
	if (!SWIG_read_ptr_array(L,index,array,size,type)){
		SWIG_Lua_pushferrstring(L,"table must contain pointers of type %s",type->name);
		SWIG_FREE_ARRAY(array);
		return 0;
	}
	return array;
}
SWIGINTERN void** SWIG_get_ptr_array_var(lua_State* L, int index, int* size,swig_type_info *type){
	void **array;
	if (!lua_istable(L,index)) {
		SWIG_Lua_pusherrstring(L,"expected a table");
		return 0;
	}
	*size=SWIG_itable_size(L,index);
	if (*size<1){
		SWIG_Lua_pusherrstring(L,"table appears to be empty");
		return 0;
	}
	array=SWIG_ALLOC_ARRAY(void*,*size);
	if (!SWIG_read_ptr_array(L,index,array,*size,type)){
		SWIG_Lua_pushferrstring(L,"table must contain pointers of type %s",type->name);
		SWIG_FREE_ARRAY(array);
		return 0;
	}
	return array;
}
SWIGINTERN void SWIG_write_ptr_array(lua_State* L,void **array,int size,swig_type_info *type,int own){
	int i;
	lua_newtable(L);
	for (i = 0; i < size; i++){
		SWIG_NewPointerObj(L,array[i],type,own);
		lua_rawseti(L,-2,i+1);/* -1 is the number, -2 is the table*/
	}
}


#include <allegro5/allegro_primitives.h>


static float *new_float(int nelements) { 
  return (float *) calloc(nelements,sizeof(float)); 
}

static void delete_float(float *ary) { 
  free(ary); 
}

static float float_getitem(float *ary, int index) {
    return ary[index];
}
static void float_setitem(float *ary, int index, float value) {
    ary[index] = value;
}

#ifdef __cplusplus
extern "C" {
#endif
static int _wrap_new_float(lua_State* L) {
  int SWIG_arg = 0;
  int arg1 ;
  float *result = 0 ;
  
  SWIG_check_num_args("new_float",1,1)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("new_float",1,"int");
  arg1 = (int)lua_tonumber(L, 1);
  result = (float *)new_float(arg1);
  SWIG_NewPointerObj(L,result,SWIGTYPE_p_float,0); SWIG_arg++; 
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_delete_float(lua_State* L) {
  int SWIG_arg = 0;
  float *arg1 = (float *) 0 ;
  
  SWIG_check_num_args("delete_float",1,1)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("delete_float",1,"float *");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_float,0))){
    SWIG_fail_ptr("delete_float",1,SWIGTYPE_p_float);
  }
  
  delete_float(arg1);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_float_getitem(lua_State* L) {
  int SWIG_arg = 0;
  float *arg1 = (float *) 0 ;
  int arg2 ;
  float result;
  
  SWIG_check_num_args("float_getitem",2,2)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("float_getitem",1,"float *");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("float_getitem",2,"int");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_float,0))){
    SWIG_fail_ptr("float_getitem",1,SWIGTYPE_p_float);
  }
  
  arg2 = (int)lua_tonumber(L, 2);
  result = (float)float_getitem(arg1,arg2);
  lua_pushnumber(L, (lua_Number) result); SWIG_arg++;
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_float_setitem(lua_State* L) {
  int SWIG_arg = 0;
  float *arg1 = (float *) 0 ;
  int arg2 ;
  float arg3 ;
  
  SWIG_check_num_args("float_setitem",3,3)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("float_setitem",1,"float *");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("float_setitem",2,"int");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("float_setitem",3,"float");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_float,0))){
    SWIG_fail_ptr("float_setitem",1,SWIGTYPE_p_float);
  }
  
  arg2 = (int)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  float_setitem(arg1,arg2,arg3);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_get_allegro_primitives_version(lua_State* L) {
  int SWIG_arg = 0;
  uint32_t result;
  
  SWIG_check_num_args("al_get_allegro_primitives_version",0,0)
  result = al_get_allegro_primitives_version();
  lua_pushnumber(L, (lua_Number) result); SWIG_arg++;
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_init_primitives_addon(lua_State* L) {
  int SWIG_arg = 0;
  bool result;
  
  SWIG_check_num_args("al_init_primitives_addon",0,0)
  result = (bool)al_init_primitives_addon();
  lua_pushboolean(L,(int)(result!=0)); SWIG_arg++;
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_shutdown_primitives_addon(lua_State* L) {
  int SWIG_arg = 0;
  
  SWIG_check_num_args("al_shutdown_primitives_addon",0,0)
  al_shutdown_primitives_addon();
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_line(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  float arg4 ;
  ALLEGRO_COLOR arg5 ;
  float arg6 ;
  ALLEGRO_COLOR *argp5 ;
  
  SWIG_check_num_args("al_draw_line",6,6)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_line",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_line",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_line",3,"float");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_line",4,"float");
  if(!lua_isuserdata(L,5)) SWIG_fail_arg("al_draw_line",5,"ALLEGRO_COLOR");
  if(!lua_isnumber(L,6)) SWIG_fail_arg("al_draw_line",6,"float");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  arg4 = (float)lua_tonumber(L, 4);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,5,(void**)&argp5,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_line",5,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg5 = *argp5;
  
  arg6 = (float)lua_tonumber(L, 6);
  al_draw_line(arg1,arg2,arg3,arg4,arg5,arg6);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_triangle(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  float arg4 ;
  float arg5 ;
  float arg6 ;
  ALLEGRO_COLOR arg7 ;
  float arg8 ;
  ALLEGRO_COLOR *argp7 ;
  
  SWIG_check_num_args("al_draw_triangle",8,8)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_triangle",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_triangle",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_triangle",3,"float");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_triangle",4,"float");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_triangle",5,"float");
  if(!lua_isnumber(L,6)) SWIG_fail_arg("al_draw_triangle",6,"float");
  if(!lua_isuserdata(L,7)) SWIG_fail_arg("al_draw_triangle",7,"ALLEGRO_COLOR");
  if(!lua_isnumber(L,8)) SWIG_fail_arg("al_draw_triangle",8,"float");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  arg4 = (float)lua_tonumber(L, 4);
  arg5 = (float)lua_tonumber(L, 5);
  arg6 = (float)lua_tonumber(L, 6);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,7,(void**)&argp7,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_triangle",7,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg7 = *argp7;
  
  arg8 = (float)lua_tonumber(L, 8);
  al_draw_triangle(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_filled_triangle(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  float arg4 ;
  float arg5 ;
  float arg6 ;
  ALLEGRO_COLOR arg7 ;
  ALLEGRO_COLOR *argp7 ;
  
  SWIG_check_num_args("al_draw_filled_triangle",7,7)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_filled_triangle",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_filled_triangle",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_filled_triangle",3,"float");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_filled_triangle",4,"float");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_filled_triangle",5,"float");
  if(!lua_isnumber(L,6)) SWIG_fail_arg("al_draw_filled_triangle",6,"float");
  if(!lua_isuserdata(L,7)) SWIG_fail_arg("al_draw_filled_triangle",7,"ALLEGRO_COLOR");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  arg4 = (float)lua_tonumber(L, 4);
  arg5 = (float)lua_tonumber(L, 5);
  arg6 = (float)lua_tonumber(L, 6);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,7,(void**)&argp7,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_filled_triangle",7,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg7 = *argp7;
  
  al_draw_filled_triangle(arg1,arg2,arg3,arg4,arg5,arg6,arg7);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_rectangle(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  float arg4 ;
  ALLEGRO_COLOR arg5 ;
  float arg6 ;
  ALLEGRO_COLOR *argp5 ;
  
  SWIG_check_num_args("al_draw_rectangle",6,6)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_rectangle",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_rectangle",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_rectangle",3,"float");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_rectangle",4,"float");
  if(!lua_isuserdata(L,5)) SWIG_fail_arg("al_draw_rectangle",5,"ALLEGRO_COLOR");
  if(!lua_isnumber(L,6)) SWIG_fail_arg("al_draw_rectangle",6,"float");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  arg4 = (float)lua_tonumber(L, 4);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,5,(void**)&argp5,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_rectangle",5,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg5 = *argp5;
  
  arg6 = (float)lua_tonumber(L, 6);
  al_draw_rectangle(arg1,arg2,arg3,arg4,arg5,arg6);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_filled_rectangle(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  float arg4 ;
  ALLEGRO_COLOR arg5 ;
  ALLEGRO_COLOR *argp5 ;
  
  SWIG_check_num_args("al_draw_filled_rectangle",5,5)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_filled_rectangle",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_filled_rectangle",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_filled_rectangle",3,"float");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_filled_rectangle",4,"float");
  if(!lua_isuserdata(L,5)) SWIG_fail_arg("al_draw_filled_rectangle",5,"ALLEGRO_COLOR");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  arg4 = (float)lua_tonumber(L, 4);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,5,(void**)&argp5,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_filled_rectangle",5,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg5 = *argp5;
  
  al_draw_filled_rectangle(arg1,arg2,arg3,arg4,arg5);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_rounded_rectangle(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  float arg4 ;
  float arg5 ;
  float arg6 ;
  ALLEGRO_COLOR arg7 ;
  float arg8 ;
  ALLEGRO_COLOR *argp7 ;
  
  SWIG_check_num_args("al_draw_rounded_rectangle",8,8)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_rounded_rectangle",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_rounded_rectangle",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_rounded_rectangle",3,"float");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_rounded_rectangle",4,"float");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_rounded_rectangle",5,"float");
  if(!lua_isnumber(L,6)) SWIG_fail_arg("al_draw_rounded_rectangle",6,"float");
  if(!lua_isuserdata(L,7)) SWIG_fail_arg("al_draw_rounded_rectangle",7,"ALLEGRO_COLOR");
  if(!lua_isnumber(L,8)) SWIG_fail_arg("al_draw_rounded_rectangle",8,"float");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  arg4 = (float)lua_tonumber(L, 4);
  arg5 = (float)lua_tonumber(L, 5);
  arg6 = (float)lua_tonumber(L, 6);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,7,(void**)&argp7,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_rounded_rectangle",7,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg7 = *argp7;
  
  arg8 = (float)lua_tonumber(L, 8);
  al_draw_rounded_rectangle(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_filled_rounded_rectangle(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  float arg4 ;
  float arg5 ;
  float arg6 ;
  ALLEGRO_COLOR arg7 ;
  ALLEGRO_COLOR *argp7 ;
  
  SWIG_check_num_args("al_draw_filled_rounded_rectangle",7,7)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_filled_rounded_rectangle",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_filled_rounded_rectangle",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_filled_rounded_rectangle",3,"float");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_filled_rounded_rectangle",4,"float");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_filled_rounded_rectangle",5,"float");
  if(!lua_isnumber(L,6)) SWIG_fail_arg("al_draw_filled_rounded_rectangle",6,"float");
  if(!lua_isuserdata(L,7)) SWIG_fail_arg("al_draw_filled_rounded_rectangle",7,"ALLEGRO_COLOR");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  arg4 = (float)lua_tonumber(L, 4);
  arg5 = (float)lua_tonumber(L, 5);
  arg6 = (float)lua_tonumber(L, 6);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,7,(void**)&argp7,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_filled_rounded_rectangle",7,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg7 = *argp7;
  
  al_draw_filled_rounded_rectangle(arg1,arg2,arg3,arg4,arg5,arg6,arg7);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap__calculate_arc(lua_State* L) {
  int SWIG_arg = 0;
  float *arg1 = (float *) 0 ;
  int arg2 ;
  float arg3 ;
  float arg4 ;
  float arg5 ;
  float arg6 ;
  float arg7 ;
  float arg8 ;
  float arg9 ;
  int arg10 ;
  
  SWIG_check_num_args("al_calculate_arc",10,10)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_calculate_arc",1,"float *");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_calculate_arc",2,"int");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_calculate_arc",3,"float");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_calculate_arc",4,"float");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_calculate_arc",5,"float");
  if(!lua_isnumber(L,6)) SWIG_fail_arg("al_calculate_arc",6,"float");
  if(!lua_isnumber(L,7)) SWIG_fail_arg("al_calculate_arc",7,"float");
  if(!lua_isnumber(L,8)) SWIG_fail_arg("al_calculate_arc",8,"float");
  if(!lua_isnumber(L,9)) SWIG_fail_arg("al_calculate_arc",9,"float");
  if(!lua_isnumber(L,10)) SWIG_fail_arg("al_calculate_arc",10,"int");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_float,0))){
    SWIG_fail_ptr("_calculate_arc",1,SWIGTYPE_p_float);
  }
  
  arg2 = (int)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  arg4 = (float)lua_tonumber(L, 4);
  arg5 = (float)lua_tonumber(L, 5);
  arg6 = (float)lua_tonumber(L, 6);
  arg7 = (float)lua_tonumber(L, 7);
  arg8 = (float)lua_tonumber(L, 8);
  arg9 = (float)lua_tonumber(L, 9);
  arg10 = (int)lua_tonumber(L, 10);
  al_calculate_arc(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_pieslice(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  float arg4 ;
  float arg5 ;
  ALLEGRO_COLOR arg6 ;
  float arg7 ;
  ALLEGRO_COLOR *argp6 ;
  
  SWIG_check_num_args("al_draw_pieslice",7,7)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_pieslice",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_pieslice",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_pieslice",3,"float");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_pieslice",4,"float");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_pieslice",5,"float");
  if(!lua_isuserdata(L,6)) SWIG_fail_arg("al_draw_pieslice",6,"ALLEGRO_COLOR");
  if(!lua_isnumber(L,7)) SWIG_fail_arg("al_draw_pieslice",7,"float");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  arg4 = (float)lua_tonumber(L, 4);
  arg5 = (float)lua_tonumber(L, 5);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,6,(void**)&argp6,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_pieslice",6,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg6 = *argp6;
  
  arg7 = (float)lua_tonumber(L, 7);
  al_draw_pieslice(arg1,arg2,arg3,arg4,arg5,arg6,arg7);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_filled_pieslice(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  float arg4 ;
  float arg5 ;
  ALLEGRO_COLOR arg6 ;
  ALLEGRO_COLOR *argp6 ;
  
  SWIG_check_num_args("al_draw_filled_pieslice",6,6)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_filled_pieslice",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_filled_pieslice",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_filled_pieslice",3,"float");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_filled_pieslice",4,"float");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_filled_pieslice",5,"float");
  if(!lua_isuserdata(L,6)) SWIG_fail_arg("al_draw_filled_pieslice",6,"ALLEGRO_COLOR");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  arg4 = (float)lua_tonumber(L, 4);
  arg5 = (float)lua_tonumber(L, 5);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,6,(void**)&argp6,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_filled_pieslice",6,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg6 = *argp6;
  
  al_draw_filled_pieslice(arg1,arg2,arg3,arg4,arg5,arg6);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_ellipse(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  float arg4 ;
  ALLEGRO_COLOR arg5 ;
  float arg6 ;
  ALLEGRO_COLOR *argp5 ;
  
  SWIG_check_num_args("al_draw_ellipse",6,6)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_ellipse",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_ellipse",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_ellipse",3,"float");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_ellipse",4,"float");
  if(!lua_isuserdata(L,5)) SWIG_fail_arg("al_draw_ellipse",5,"ALLEGRO_COLOR");
  if(!lua_isnumber(L,6)) SWIG_fail_arg("al_draw_ellipse",6,"float");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  arg4 = (float)lua_tonumber(L, 4);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,5,(void**)&argp5,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_ellipse",5,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg5 = *argp5;
  
  arg6 = (float)lua_tonumber(L, 6);
  al_draw_ellipse(arg1,arg2,arg3,arg4,arg5,arg6);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_filled_ellipse(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  float arg4 ;
  ALLEGRO_COLOR arg5 ;
  ALLEGRO_COLOR *argp5 ;
  
  SWIG_check_num_args("al_draw_filled_ellipse",5,5)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_filled_ellipse",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_filled_ellipse",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_filled_ellipse",3,"float");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_filled_ellipse",4,"float");
  if(!lua_isuserdata(L,5)) SWIG_fail_arg("al_draw_filled_ellipse",5,"ALLEGRO_COLOR");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  arg4 = (float)lua_tonumber(L, 4);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,5,(void**)&argp5,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_filled_ellipse",5,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg5 = *argp5;
  
  al_draw_filled_ellipse(arg1,arg2,arg3,arg4,arg5);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_circle(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  ALLEGRO_COLOR arg4 ;
  float arg5 ;
  ALLEGRO_COLOR *argp4 ;
  
  SWIG_check_num_args("al_draw_circle",5,5)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_circle",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_circle",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_circle",3,"float");
  if(!lua_isuserdata(L,4)) SWIG_fail_arg("al_draw_circle",4,"ALLEGRO_COLOR");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_circle",5,"float");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,4,(void**)&argp4,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_circle",4,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg4 = *argp4;
  
  arg5 = (float)lua_tonumber(L, 5);
  al_draw_circle(arg1,arg2,arg3,arg4,arg5);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_filled_circle(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  ALLEGRO_COLOR arg4 ;
  ALLEGRO_COLOR *argp4 ;
  
  SWIG_check_num_args("al_draw_filled_circle",4,4)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_filled_circle",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_filled_circle",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_filled_circle",3,"float");
  if(!lua_isuserdata(L,4)) SWIG_fail_arg("al_draw_filled_circle",4,"ALLEGRO_COLOR");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,4,(void**)&argp4,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_filled_circle",4,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg4 = *argp4;
  
  al_draw_filled_circle(arg1,arg2,arg3,arg4);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_arc(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  float arg4 ;
  float arg5 ;
  ALLEGRO_COLOR arg6 ;
  float arg7 ;
  ALLEGRO_COLOR *argp6 ;
  
  SWIG_check_num_args("al_draw_arc",7,7)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_arc",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_arc",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_arc",3,"float");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_arc",4,"float");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_arc",5,"float");
  if(!lua_isuserdata(L,6)) SWIG_fail_arg("al_draw_arc",6,"ALLEGRO_COLOR");
  if(!lua_isnumber(L,7)) SWIG_fail_arg("al_draw_arc",7,"float");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  arg4 = (float)lua_tonumber(L, 4);
  arg5 = (float)lua_tonumber(L, 5);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,6,(void**)&argp6,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_arc",6,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg6 = *argp6;
  
  arg7 = (float)lua_tonumber(L, 7);
  al_draw_arc(arg1,arg2,arg3,arg4,arg5,arg6,arg7);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_elliptical_arc(lua_State* L) {
  int SWIG_arg = 0;
  float arg1 ;
  float arg2 ;
  float arg3 ;
  float arg4 ;
  float arg5 ;
  float arg6 ;
  ALLEGRO_COLOR arg7 ;
  float arg8 ;
  ALLEGRO_COLOR *argp7 ;
  
  SWIG_check_num_args("al_draw_elliptical_arc",8,8)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_draw_elliptical_arc",1,"float");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_elliptical_arc",2,"float");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_elliptical_arc",3,"float");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_elliptical_arc",4,"float");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_elliptical_arc",5,"float");
  if(!lua_isnumber(L,6)) SWIG_fail_arg("al_draw_elliptical_arc",6,"float");
  if(!lua_isuserdata(L,7)) SWIG_fail_arg("al_draw_elliptical_arc",7,"ALLEGRO_COLOR");
  if(!lua_isnumber(L,8)) SWIG_fail_arg("al_draw_elliptical_arc",8,"float");
  arg1 = (float)lua_tonumber(L, 1);
  arg2 = (float)lua_tonumber(L, 2);
  arg3 = (float)lua_tonumber(L, 3);
  arg4 = (float)lua_tonumber(L, 4);
  arg5 = (float)lua_tonumber(L, 5);
  arg6 = (float)lua_tonumber(L, 6);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,7,(void**)&argp7,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_elliptical_arc",7,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg7 = *argp7;
  
  arg8 = (float)lua_tonumber(L, 8);
  al_draw_elliptical_arc(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap__calculate_spline(lua_State* L) {
  int SWIG_arg = 0;
  float *arg1 = (float *) 0 ;
  int arg2 ;
  float *arg3 ;
  float arg4 ;
  int arg5 ;
  float temp3[8] ;
  
  SWIG_check_num_args("al_calculate_spline",5,5)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_calculate_spline",1,"float *");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_calculate_spline",2,"int");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_calculate_spline",4,"float");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_calculate_spline",5,"int");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_float,0))){
    SWIG_fail_ptr("_calculate_spline",1,SWIGTYPE_p_float);
  }
  
  arg2 = (int)lua_tonumber(L, 2);
  {
    int i, size;
    // assert length
    lua_len (L, 3);
    size = lua_tointeger (L, -1);
    if (size != 8) {
      luaL_error (L, "expected %d floats in table, not %d", 8, size);
    }
    for (i = 0; i < 8; i++) {
      lua_geti (L, 3, i + 1);
      temp3[i] = luaL_checknumber (L, -1);
    }
    lua_pop (L, 8 + 1);
    arg3 = temp3;
  }
  arg4 = (float)lua_tonumber(L, 4);
  arg5 = (int)lua_tonumber(L, 5);
  al_calculate_spline(arg1,arg2,arg3,arg4,arg5);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_spline(lua_State* L) {
  int SWIG_arg = 0;
  float *arg1 ;
  ALLEGRO_COLOR arg2 ;
  float arg3 ;
  float temp1[8] ;
  ALLEGRO_COLOR *argp2 ;
  
  SWIG_check_num_args("al_draw_spline",3,3)
  if(!lua_isuserdata(L,2)) SWIG_fail_arg("al_draw_spline",2,"ALLEGRO_COLOR");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_spline",3,"float");
  {
    int i, size;
    // assert length
    lua_len (L, 1);
    size = lua_tointeger (L, -1);
    if (size != 8) {
      luaL_error (L, "expected %d floats in table, not %d", 8, size);
    }
    for (i = 0; i < 8; i++) {
      lua_geti (L, 1, i + 1);
      temp1[i] = luaL_checknumber (L, -1);
    }
    lua_pop (L, 8 + 1);
    arg1 = temp1;
  }
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,2,(void**)&argp2,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("draw_spline",2,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg2 = *argp2;
  
  arg3 = (float)lua_tonumber(L, 3);
  al_draw_spline(arg1,arg2,arg3);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap__calculate_ribbon(lua_State* L) {
  int SWIG_arg = 0;
  float *arg1 = (float *) 0 ;
  int arg2 ;
  float *arg3 = (float *) 0 ;
  int arg4 ;
  float arg5 ;
  int arg6 ;
  
  SWIG_check_num_args("al_calculate_ribbon",6,6)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_calculate_ribbon",1,"float *");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_calculate_ribbon",2,"int");
  if(!SWIG_isptrtype(L,3)) SWIG_fail_arg("al_calculate_ribbon",3,"float const *");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_calculate_ribbon",4,"int");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_calculate_ribbon",5,"float");
  if(!lua_isnumber(L,6)) SWIG_fail_arg("al_calculate_ribbon",6,"int");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_float,0))){
    SWIG_fail_ptr("_calculate_ribbon",1,SWIGTYPE_p_float);
  }
  
  arg2 = (int)lua_tonumber(L, 2);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,3,(void**)&arg3,SWIGTYPE_p_float,0))){
    SWIG_fail_ptr("_calculate_ribbon",3,SWIGTYPE_p_float);
  }
  
  arg4 = (int)lua_tonumber(L, 4);
  arg5 = (float)lua_tonumber(L, 5);
  arg6 = (int)lua_tonumber(L, 6);
  al_calculate_ribbon(arg1,arg2,(float const *)arg3,arg4,arg5,arg6);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap__draw_ribbon(lua_State* L) {
  int SWIG_arg = 0;
  float *arg1 = (float *) 0 ;
  int arg2 ;
  ALLEGRO_COLOR arg3 ;
  float arg4 ;
  int arg5 ;
  ALLEGRO_COLOR *argp3 ;
  
  SWIG_check_num_args("al_draw_ribbon",5,5)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_draw_ribbon",1,"float const *");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_ribbon",2,"int");
  if(!lua_isuserdata(L,3)) SWIG_fail_arg("al_draw_ribbon",3,"ALLEGRO_COLOR");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_ribbon",4,"float");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_ribbon",5,"int");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_float,0))){
    SWIG_fail_ptr("_draw_ribbon",1,SWIGTYPE_p_float);
  }
  
  arg2 = (int)lua_tonumber(L, 2);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,3,(void**)&argp3,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("_draw_ribbon",3,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg3 = *argp3;
  
  arg4 = (float)lua_tonumber(L, 4);
  arg5 = (int)lua_tonumber(L, 5);
  al_draw_ribbon((float const *)arg1,arg2,arg3,arg4,arg5);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_prim(lua_State* L) {
  int SWIG_arg = 0;
  void *arg1 = (void *) 0 ;
  ALLEGRO_VERTEX_DECL *arg2 = (ALLEGRO_VERTEX_DECL *) 0 ;
  ALLEGRO_BITMAP *arg3 = (ALLEGRO_BITMAP *) 0 ;
  int arg4 ;
  int arg5 ;
  int arg6 ;
  int result;
  
  SWIG_check_num_args("al_draw_prim",6,6)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_draw_prim",1,"void const *");
  if(!SWIG_isptrtype(L,2)) SWIG_fail_arg("al_draw_prim",2,"ALLEGRO_VERTEX_DECL const *");
  if(!SWIG_isptrtype(L,3)) SWIG_fail_arg("al_draw_prim",3,"ALLEGRO_BITMAP *");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_prim",4,"int");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_prim",5,"int");
  if(!lua_isnumber(L,6)) SWIG_fail_arg("al_draw_prim",6,"int");
  arg1=(void *)SWIG_MustGetPtr(L,1,0,0,1,"draw_prim");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,2,(void**)&arg2,SWIGTYPE_p_ALLEGRO_VERTEX_DECL,0))){
    SWIG_fail_ptr("draw_prim",2,SWIGTYPE_p_ALLEGRO_VERTEX_DECL);
  }
  
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,3,(void**)&arg3,SWIGTYPE_p_ALLEGRO_BITMAP,0))){
    SWIG_fail_ptr("draw_prim",3,SWIGTYPE_p_ALLEGRO_BITMAP);
  }
  
  arg4 = (int)lua_tonumber(L, 4);
  arg5 = (int)lua_tonumber(L, 5);
  arg6 = (int)lua_tonumber(L, 6);
  result = (int)al_draw_prim((void const *)arg1,(struct ALLEGRO_VERTEX_DECL const *)arg2,arg3,arg4,arg5,arg6);
  lua_pushnumber(L, (lua_Number) result); SWIG_arg++;
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_indexed_prim(lua_State* L) {
  int SWIG_arg = 0;
  void *arg1 = (void *) 0 ;
  ALLEGRO_VERTEX_DECL *arg2 = (ALLEGRO_VERTEX_DECL *) 0 ;
  ALLEGRO_BITMAP *arg3 = (ALLEGRO_BITMAP *) 0 ;
  int *arg4 = (int *) 0 ;
  int arg5 ;
  int arg6 ;
  int result;
  
  SWIG_check_num_args("al_draw_indexed_prim",5,5)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_draw_indexed_prim",1,"void const *");
  if(!SWIG_isptrtype(L,2)) SWIG_fail_arg("al_draw_indexed_prim",2,"ALLEGRO_VERTEX_DECL const *");
  if(!SWIG_isptrtype(L,3)) SWIG_fail_arg("al_draw_indexed_prim",3,"ALLEGRO_BITMAP *");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_indexed_prim",5,"int");
  arg1=(void *)SWIG_MustGetPtr(L,1,0,0,1,"draw_indexed_prim");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,2,(void**)&arg2,SWIGTYPE_p_ALLEGRO_VERTEX_DECL,0))){
    SWIG_fail_ptr("draw_indexed_prim",2,SWIGTYPE_p_ALLEGRO_VERTEX_DECL);
  }
  
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,3,(void**)&arg3,SWIGTYPE_p_ALLEGRO_BITMAP,0))){
    SWIG_fail_ptr("draw_indexed_prim",3,SWIGTYPE_p_ALLEGRO_BITMAP);
  }
  
  arg4 = SWIG_get_int_num_array_var(L,4,&arg5);
  if (!arg4) SWIG_fail;
  arg6 = (int)lua_tonumber(L, 5);
  result = (int)al_draw_indexed_prim((void const *)arg1,(struct ALLEGRO_VERTEX_DECL const *)arg2,arg3,(int const *)arg4,arg5,arg6);
  lua_pushnumber(L, (lua_Number) result); SWIG_arg++;
  SWIG_FREE_ARRAY(arg4);
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  SWIG_FREE_ARRAY(arg4);
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_vertex_buffer(lua_State* L) {
  int SWIG_arg = 0;
  ALLEGRO_VERTEX_BUFFER *arg1 = (ALLEGRO_VERTEX_BUFFER *) 0 ;
  ALLEGRO_BITMAP *arg2 = (ALLEGRO_BITMAP *) 0 ;
  int arg3 ;
  int arg4 ;
  int arg5 ;
  int result;
  
  SWIG_check_num_args("al_draw_vertex_buffer",5,5)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_draw_vertex_buffer",1,"ALLEGRO_VERTEX_BUFFER *");
  if(!SWIG_isptrtype(L,2)) SWIG_fail_arg("al_draw_vertex_buffer",2,"ALLEGRO_BITMAP *");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_vertex_buffer",3,"int");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_vertex_buffer",4,"int");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_vertex_buffer",5,"int");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_ALLEGRO_VERTEX_BUFFER,0))){
    SWIG_fail_ptr("draw_vertex_buffer",1,SWIGTYPE_p_ALLEGRO_VERTEX_BUFFER);
  }
  
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,2,(void**)&arg2,SWIGTYPE_p_ALLEGRO_BITMAP,0))){
    SWIG_fail_ptr("draw_vertex_buffer",2,SWIGTYPE_p_ALLEGRO_BITMAP);
  }
  
  arg3 = (int)lua_tonumber(L, 3);
  arg4 = (int)lua_tonumber(L, 4);
  arg5 = (int)lua_tonumber(L, 5);
  result = (int)al_draw_vertex_buffer(arg1,arg2,arg3,arg4,arg5);
  lua_pushnumber(L, (lua_Number) result); SWIG_arg++;
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_draw_indexed_buffer(lua_State* L) {
  int SWIG_arg = 0;
  ALLEGRO_VERTEX_BUFFER *arg1 = (ALLEGRO_VERTEX_BUFFER *) 0 ;
  ALLEGRO_BITMAP *arg2 = (ALLEGRO_BITMAP *) 0 ;
  ALLEGRO_INDEX_BUFFER *arg3 = (ALLEGRO_INDEX_BUFFER *) 0 ;
  int arg4 ;
  int arg5 ;
  int arg6 ;
  int result;
  
  SWIG_check_num_args("al_draw_indexed_buffer",6,6)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_draw_indexed_buffer",1,"ALLEGRO_VERTEX_BUFFER *");
  if(!SWIG_isptrtype(L,2)) SWIG_fail_arg("al_draw_indexed_buffer",2,"ALLEGRO_BITMAP *");
  if(!SWIG_isptrtype(L,3)) SWIG_fail_arg("al_draw_indexed_buffer",3,"ALLEGRO_INDEX_BUFFER *");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_indexed_buffer",4,"int");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_indexed_buffer",5,"int");
  if(!lua_isnumber(L,6)) SWIG_fail_arg("al_draw_indexed_buffer",6,"int");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_ALLEGRO_VERTEX_BUFFER,0))){
    SWIG_fail_ptr("draw_indexed_buffer",1,SWIGTYPE_p_ALLEGRO_VERTEX_BUFFER);
  }
  
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,2,(void**)&arg2,SWIGTYPE_p_ALLEGRO_BITMAP,0))){
    SWIG_fail_ptr("draw_indexed_buffer",2,SWIGTYPE_p_ALLEGRO_BITMAP);
  }
  
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,3,(void**)&arg3,SWIGTYPE_p_ALLEGRO_INDEX_BUFFER,0))){
    SWIG_fail_ptr("draw_indexed_buffer",3,SWIGTYPE_p_ALLEGRO_INDEX_BUFFER);
  }
  
  arg4 = (int)lua_tonumber(L, 4);
  arg5 = (int)lua_tonumber(L, 5);
  arg6 = (int)lua_tonumber(L, 6);
  result = (int)al_draw_indexed_buffer(arg1,arg2,arg3,arg4,arg5,arg6);
  lua_pushnumber(L, (lua_Number) result); SWIG_arg++;
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_create_vertex_decl(lua_State* L) {
  int SWIG_arg = 0;
  ALLEGRO_VERTEX_ELEMENT *arg1 = (ALLEGRO_VERTEX_ELEMENT *) 0 ;
  int arg2 ;
  ALLEGRO_VERTEX_DECL *result = 0 ;
  
  SWIG_check_num_args("al_create_vertex_decl",2,2)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_create_vertex_decl",1,"ALLEGRO_VERTEX_ELEMENT const *");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_create_vertex_decl",2,"int");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_ALLEGRO_VERTEX_ELEMENT,0))){
    SWIG_fail_ptr("create_vertex_decl",1,SWIGTYPE_p_ALLEGRO_VERTEX_ELEMENT);
  }
  
  arg2 = (int)lua_tonumber(L, 2);
  result = (ALLEGRO_VERTEX_DECL *)al_create_vertex_decl((struct ALLEGRO_VERTEX_ELEMENT const *)arg1,arg2);
  SWIG_NewPointerObj(L,result,SWIGTYPE_p_ALLEGRO_VERTEX_DECL,0); SWIG_arg++; 
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_destroy_vertex_decl(lua_State* L) {
  int SWIG_arg = 0;
  ALLEGRO_VERTEX_DECL *arg1 = (ALLEGRO_VERTEX_DECL *) 0 ;
  
  SWIG_check_num_args("al_destroy_vertex_decl",1,1)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_destroy_vertex_decl",1,"ALLEGRO_VERTEX_DECL *");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_ALLEGRO_VERTEX_DECL,0))){
    SWIG_fail_ptr("destroy_vertex_decl",1,SWIGTYPE_p_ALLEGRO_VERTEX_DECL);
  }
  
  al_destroy_vertex_decl(arg1);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_create_vertex_buffer(lua_State* L) {
  int SWIG_arg = 0;
  ALLEGRO_VERTEX_DECL *arg1 = (ALLEGRO_VERTEX_DECL *) 0 ;
  void *arg2 = (void *) 0 ;
  int arg3 ;
  int arg4 ;
  ALLEGRO_VERTEX_BUFFER *result = 0 ;
  
  SWIG_check_num_args("al_create_vertex_buffer",4,4)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_create_vertex_buffer",1,"ALLEGRO_VERTEX_DECL *");
  if(!SWIG_isptrtype(L,2)) SWIG_fail_arg("al_create_vertex_buffer",2,"void const *");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_create_vertex_buffer",3,"int");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_create_vertex_buffer",4,"int");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_ALLEGRO_VERTEX_DECL,0))){
    SWIG_fail_ptr("create_vertex_buffer",1,SWIGTYPE_p_ALLEGRO_VERTEX_DECL);
  }
  
  arg2=(void *)SWIG_MustGetPtr(L,2,0,0,2,"create_vertex_buffer");
  arg3 = (int)lua_tonumber(L, 3);
  arg4 = (int)lua_tonumber(L, 4);
  result = (ALLEGRO_VERTEX_BUFFER *)al_create_vertex_buffer(arg1,(void const *)arg2,arg3,arg4);
  SWIG_NewPointerObj(L,result,SWIGTYPE_p_ALLEGRO_VERTEX_BUFFER,0); SWIG_arg++; 
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_destroy_vertex_buffer(lua_State* L) {
  int SWIG_arg = 0;
  ALLEGRO_VERTEX_BUFFER *arg1 = (ALLEGRO_VERTEX_BUFFER *) 0 ;
  
  SWIG_check_num_args("al_destroy_vertex_buffer",1,1)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_destroy_vertex_buffer",1,"ALLEGRO_VERTEX_BUFFER *");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_ALLEGRO_VERTEX_BUFFER,0))){
    SWIG_fail_ptr("destroy_vertex_buffer",1,SWIGTYPE_p_ALLEGRO_VERTEX_BUFFER);
  }
  
  al_destroy_vertex_buffer(arg1);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_lock_vertex_buffer(lua_State* L) {
  int SWIG_arg = 0;
  ALLEGRO_VERTEX_BUFFER *arg1 = (ALLEGRO_VERTEX_BUFFER *) 0 ;
  int arg2 ;
  int arg3 ;
  int arg4 ;
  void *result = 0 ;
  
  SWIG_check_num_args("al_lock_vertex_buffer",4,4)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_lock_vertex_buffer",1,"ALLEGRO_VERTEX_BUFFER *");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_lock_vertex_buffer",2,"int");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_lock_vertex_buffer",3,"int");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_lock_vertex_buffer",4,"int");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_ALLEGRO_VERTEX_BUFFER,0))){
    SWIG_fail_ptr("lock_vertex_buffer",1,SWIGTYPE_p_ALLEGRO_VERTEX_BUFFER);
  }
  
  arg2 = (int)lua_tonumber(L, 2);
  arg3 = (int)lua_tonumber(L, 3);
  arg4 = (int)lua_tonumber(L, 4);
  result = (void *)al_lock_vertex_buffer(arg1,arg2,arg3,arg4);
  SWIG_NewPointerObj(L,result,SWIGTYPE_p_void,0); SWIG_arg++; 
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_unlock_vertex_buffer(lua_State* L) {
  int SWIG_arg = 0;
  ALLEGRO_VERTEX_BUFFER *arg1 = (ALLEGRO_VERTEX_BUFFER *) 0 ;
  
  SWIG_check_num_args("al_unlock_vertex_buffer",1,1)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_unlock_vertex_buffer",1,"ALLEGRO_VERTEX_BUFFER *");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_ALLEGRO_VERTEX_BUFFER,0))){
    SWIG_fail_ptr("unlock_vertex_buffer",1,SWIGTYPE_p_ALLEGRO_VERTEX_BUFFER);
  }
  
  al_unlock_vertex_buffer(arg1);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_get_vertex_buffer_size(lua_State* L) {
  int SWIG_arg = 0;
  ALLEGRO_VERTEX_BUFFER *arg1 = (ALLEGRO_VERTEX_BUFFER *) 0 ;
  int result;
  
  SWIG_check_num_args("al_get_vertex_buffer_size",1,1)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_get_vertex_buffer_size",1,"ALLEGRO_VERTEX_BUFFER *");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_ALLEGRO_VERTEX_BUFFER,0))){
    SWIG_fail_ptr("get_vertex_buffer_size",1,SWIGTYPE_p_ALLEGRO_VERTEX_BUFFER);
  }
  
  result = (int)al_get_vertex_buffer_size(arg1);
  lua_pushnumber(L, (lua_Number) result); SWIG_arg++;
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_create_index_buffer(lua_State* L) {
  int SWIG_arg = 0;
  int arg1 ;
  void *arg2 = (void *) 0 ;
  int arg3 ;
  int arg4 ;
  ALLEGRO_INDEX_BUFFER *result = 0 ;
  
  SWIG_check_num_args("al_create_index_buffer",4,4)
  if(!lua_isnumber(L,1)) SWIG_fail_arg("al_create_index_buffer",1,"int");
  if(!SWIG_isptrtype(L,2)) SWIG_fail_arg("al_create_index_buffer",2,"void const *");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_create_index_buffer",3,"int");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_create_index_buffer",4,"int");
  arg1 = (int)lua_tonumber(L, 1);
  arg2=(void *)SWIG_MustGetPtr(L,2,0,0,2,"create_index_buffer");
  arg3 = (int)lua_tonumber(L, 3);
  arg4 = (int)lua_tonumber(L, 4);
  result = (ALLEGRO_INDEX_BUFFER *)al_create_index_buffer(arg1,(void const *)arg2,arg3,arg4);
  SWIG_NewPointerObj(L,result,SWIGTYPE_p_ALLEGRO_INDEX_BUFFER,0); SWIG_arg++; 
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_destroy_index_buffer(lua_State* L) {
  int SWIG_arg = 0;
  ALLEGRO_INDEX_BUFFER *arg1 = (ALLEGRO_INDEX_BUFFER *) 0 ;
  
  SWIG_check_num_args("al_destroy_index_buffer",1,1)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_destroy_index_buffer",1,"ALLEGRO_INDEX_BUFFER *");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_ALLEGRO_INDEX_BUFFER,0))){
    SWIG_fail_ptr("destroy_index_buffer",1,SWIGTYPE_p_ALLEGRO_INDEX_BUFFER);
  }
  
  al_destroy_index_buffer(arg1);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_lock_index_buffer(lua_State* L) {
  int SWIG_arg = 0;
  ALLEGRO_INDEX_BUFFER *arg1 = (ALLEGRO_INDEX_BUFFER *) 0 ;
  int arg2 ;
  int arg3 ;
  int arg4 ;
  void *result = 0 ;
  
  SWIG_check_num_args("al_lock_index_buffer",4,4)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_lock_index_buffer",1,"ALLEGRO_INDEX_BUFFER *");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_lock_index_buffer",2,"int");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_lock_index_buffer",3,"int");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_lock_index_buffer",4,"int");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_ALLEGRO_INDEX_BUFFER,0))){
    SWIG_fail_ptr("lock_index_buffer",1,SWIGTYPE_p_ALLEGRO_INDEX_BUFFER);
  }
  
  arg2 = (int)lua_tonumber(L, 2);
  arg3 = (int)lua_tonumber(L, 3);
  arg4 = (int)lua_tonumber(L, 4);
  result = (void *)al_lock_index_buffer(arg1,arg2,arg3,arg4);
  SWIG_NewPointerObj(L,result,SWIGTYPE_p_void,0); SWIG_arg++; 
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_unlock_index_buffer(lua_State* L) {
  int SWIG_arg = 0;
  ALLEGRO_INDEX_BUFFER *arg1 = (ALLEGRO_INDEX_BUFFER *) 0 ;
  
  SWIG_check_num_args("al_unlock_index_buffer",1,1)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_unlock_index_buffer",1,"ALLEGRO_INDEX_BUFFER *");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_ALLEGRO_INDEX_BUFFER,0))){
    SWIG_fail_ptr("unlock_index_buffer",1,SWIGTYPE_p_ALLEGRO_INDEX_BUFFER);
  }
  
  al_unlock_index_buffer(arg1);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap_get_index_buffer_size(lua_State* L) {
  int SWIG_arg = 0;
  ALLEGRO_INDEX_BUFFER *arg1 = (ALLEGRO_INDEX_BUFFER *) 0 ;
  int result;
  
  SWIG_check_num_args("al_get_index_buffer_size",1,1)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_get_index_buffer_size",1,"ALLEGRO_INDEX_BUFFER *");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_ALLEGRO_INDEX_BUFFER,0))){
    SWIG_fail_ptr("get_index_buffer_size",1,SWIGTYPE_p_ALLEGRO_INDEX_BUFFER);
  }
  
  result = (int)al_get_index_buffer_size(arg1);
  lua_pushnumber(L, (lua_Number) result); SWIG_arg++;
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap__draw_polyline(lua_State* L) {
  int SWIG_arg = 0;
  float *arg1 = (float *) 0 ;
  int arg2 ;
  int arg3 ;
  int arg4 ;
  int arg5 ;
  ALLEGRO_COLOR arg6 ;
  float arg7 ;
  float arg8 ;
  ALLEGRO_COLOR *argp6 ;
  
  SWIG_check_num_args("al_draw_polyline",8,8)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_draw_polyline",1,"float const *");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_polyline",2,"int");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_polyline",3,"int");
  if(!lua_isnumber(L,4)) SWIG_fail_arg("al_draw_polyline",4,"int");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_polyline",5,"int");
  if(!lua_isuserdata(L,6)) SWIG_fail_arg("al_draw_polyline",6,"ALLEGRO_COLOR");
  if(!lua_isnumber(L,7)) SWIG_fail_arg("al_draw_polyline",7,"float");
  if(!lua_isnumber(L,8)) SWIG_fail_arg("al_draw_polyline",8,"float");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_float,0))){
    SWIG_fail_ptr("_draw_polyline",1,SWIGTYPE_p_float);
  }
  
  arg2 = (int)lua_tonumber(L, 2);
  arg3 = (int)lua_tonumber(L, 3);
  arg4 = (int)lua_tonumber(L, 4);
  arg5 = (int)lua_tonumber(L, 5);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,6,(void**)&argp6,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("_draw_polyline",6,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg6 = *argp6;
  
  arg7 = (float)lua_tonumber(L, 7);
  arg8 = (float)lua_tonumber(L, 8);
  al_draw_polyline((float const *)arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap__draw_polygon(lua_State* L) {
  int SWIG_arg = 0;
  float *arg1 = (float *) 0 ;
  int arg2 ;
  int arg3 ;
  ALLEGRO_COLOR arg4 ;
  float arg5 ;
  float arg6 ;
  ALLEGRO_COLOR *argp4 ;
  
  SWIG_check_num_args("al_draw_polygon",6,6)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_draw_polygon",1,"float const *");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_polygon",2,"int");
  if(!lua_isnumber(L,3)) SWIG_fail_arg("al_draw_polygon",3,"int");
  if(!lua_isuserdata(L,4)) SWIG_fail_arg("al_draw_polygon",4,"ALLEGRO_COLOR");
  if(!lua_isnumber(L,5)) SWIG_fail_arg("al_draw_polygon",5,"float");
  if(!lua_isnumber(L,6)) SWIG_fail_arg("al_draw_polygon",6,"float");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_float,0))){
    SWIG_fail_ptr("_draw_polygon",1,SWIGTYPE_p_float);
  }
  
  arg2 = (int)lua_tonumber(L, 2);
  arg3 = (int)lua_tonumber(L, 3);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,4,(void**)&argp4,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("_draw_polygon",4,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg4 = *argp4;
  
  arg5 = (float)lua_tonumber(L, 5);
  arg6 = (float)lua_tonumber(L, 6);
  al_draw_polygon((float const *)arg1,arg2,arg3,arg4,arg5,arg6);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap__draw_filled_polygon(lua_State* L) {
  int SWIG_arg = 0;
  float *arg1 = (float *) 0 ;
  int arg2 ;
  ALLEGRO_COLOR arg3 ;
  ALLEGRO_COLOR *argp3 ;
  
  SWIG_check_num_args("al_draw_filled_polygon",3,3)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_draw_filled_polygon",1,"float const *");
  if(!lua_isnumber(L,2)) SWIG_fail_arg("al_draw_filled_polygon",2,"int");
  if(!lua_isuserdata(L,3)) SWIG_fail_arg("al_draw_filled_polygon",3,"ALLEGRO_COLOR");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_float,0))){
    SWIG_fail_ptr("_draw_filled_polygon",1,SWIGTYPE_p_float);
  }
  
  arg2 = (int)lua_tonumber(L, 2);
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,3,(void**)&argp3,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("_draw_filled_polygon",3,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg3 = *argp3;
  
  al_draw_filled_polygon((float const *)arg1,arg2,arg3);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static int _wrap__draw_filled_polygon_with_holes(lua_State* L) {
  int SWIG_arg = 0;
  float *arg1 = (float *) 0 ;
  int *arg2 = (int *) 0 ;
  ALLEGRO_COLOR arg3 ;
  ALLEGRO_COLOR *argp3 ;
  
  SWIG_check_num_args("al_draw_filled_polygon_with_holes",3,3)
  if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("al_draw_filled_polygon_with_holes",1,"float const *");
  if(!SWIG_isptrtype(L,2)) SWIG_fail_arg("al_draw_filled_polygon_with_holes",2,"int const *");
  if(!lua_isuserdata(L,3)) SWIG_fail_arg("al_draw_filled_polygon_with_holes",3,"ALLEGRO_COLOR");
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_float,0))){
    SWIG_fail_ptr("_draw_filled_polygon_with_holes",1,SWIGTYPE_p_float);
  }
  
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,2,(void**)&arg2,SWIGTYPE_p_int,0))){
    SWIG_fail_ptr("_draw_filled_polygon_with_holes",2,SWIGTYPE_p_int);
  }
  
  
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,3,(void**)&argp3,SWIGTYPE_p_ALLEGRO_COLOR,0))){
    SWIG_fail_ptr("_draw_filled_polygon_with_holes",3,SWIGTYPE_p_ALLEGRO_COLOR);
  }
  arg3 = *argp3;
  
  al_draw_filled_polygon_with_holes((float const *)arg1,(int const *)arg2,arg3);
  
  return SWIG_arg;
  
  if(0) SWIG_fail;
  
fail:
  lua_error(L);
  return SWIG_arg;
}


static swig_lua_attribute swig_SwigModule_attributes[] = {
    {0,0,0}
};
static swig_lua_const_info swig_SwigModule_constants[]= {
    {SWIG_LUA_CONSTTAB_INT("float_size", sizeof(float))},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_LINE_LIST", ALLEGRO_PRIM_LINE_LIST)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_LINE_STRIP", ALLEGRO_PRIM_LINE_STRIP)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_LINE_LOOP", ALLEGRO_PRIM_LINE_LOOP)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_TRIANGLE_LIST", ALLEGRO_PRIM_TRIANGLE_LIST)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_TRIANGLE_STRIP", ALLEGRO_PRIM_TRIANGLE_STRIP)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_TRIANGLE_FAN", ALLEGRO_PRIM_TRIANGLE_FAN)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_POINT_LIST", ALLEGRO_PRIM_POINT_LIST)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_NUM_TYPES", ALLEGRO_PRIM_NUM_TYPES)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_MAX_USER_ATTR", ALLEGRO_PRIM_MAX_USER_ATTR)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_POSITION", ALLEGRO_PRIM_POSITION)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_COLOR_ATTR", ALLEGRO_PRIM_COLOR_ATTR)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_TEX_COORD", ALLEGRO_PRIM_TEX_COORD)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_TEX_COORD_PIXEL", ALLEGRO_PRIM_TEX_COORD_PIXEL)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_USER_ATTR", ALLEGRO_PRIM_USER_ATTR)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_ATTR_NUM", ALLEGRO_PRIM_ATTR_NUM)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_FLOAT_2", ALLEGRO_PRIM_FLOAT_2)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_FLOAT_3", ALLEGRO_PRIM_FLOAT_3)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_SHORT_2", ALLEGRO_PRIM_SHORT_2)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_FLOAT_1", ALLEGRO_PRIM_FLOAT_1)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_FLOAT_4", ALLEGRO_PRIM_FLOAT_4)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_UBYTE_4", ALLEGRO_PRIM_UBYTE_4)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_SHORT_4", ALLEGRO_PRIM_SHORT_4)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_NORMALIZED_UBYTE_4", ALLEGRO_PRIM_NORMALIZED_UBYTE_4)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_NORMALIZED_SHORT_2", ALLEGRO_PRIM_NORMALIZED_SHORT_2)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_NORMALIZED_SHORT_4", ALLEGRO_PRIM_NORMALIZED_SHORT_4)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_NORMALIZED_USHORT_2", ALLEGRO_PRIM_NORMALIZED_USHORT_2)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_NORMALIZED_USHORT_4", ALLEGRO_PRIM_NORMALIZED_USHORT_4)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_HALF_FLOAT_2", ALLEGRO_PRIM_HALF_FLOAT_2)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_HALF_FLOAT_4", ALLEGRO_PRIM_HALF_FLOAT_4)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_LINE_JOIN_NONE", ALLEGRO_LINE_JOIN_NONE)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_LINE_JOIN_BEVEL", ALLEGRO_LINE_JOIN_BEVEL)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_LINE_JOIN_ROUND", ALLEGRO_LINE_JOIN_ROUND)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_LINE_JOIN_MITER", ALLEGRO_LINE_JOIN_MITER)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_LINE_JOIN_MITRE", ALLEGRO_LINE_JOIN_MITRE)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_LINE_CAP_NONE", ALLEGRO_LINE_CAP_NONE)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_LINE_CAP_SQUARE", ALLEGRO_LINE_CAP_SQUARE)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_LINE_CAP_ROUND", ALLEGRO_LINE_CAP_ROUND)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_LINE_CAP_TRIANGLE", ALLEGRO_LINE_CAP_TRIANGLE)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_LINE_CAP_CLOSED", ALLEGRO_LINE_CAP_CLOSED)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_BUFFER_STREAM", ALLEGRO_PRIM_BUFFER_STREAM)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_BUFFER_STATIC", ALLEGRO_PRIM_BUFFER_STATIC)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_BUFFER_DYNAMIC", ALLEGRO_PRIM_BUFFER_DYNAMIC)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_BUFFER_READWRITE", ALLEGRO_PRIM_BUFFER_READWRITE)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_VERTEX_CACHE_SIZE", 256)},
    {SWIG_LUA_CONSTTAB_INT("ALLEGRO_PRIM_QUALITY", 10)},
    {0,0,0,0,0,0}
};
static swig_lua_method swig_SwigModule_methods[]= {
    { "new_float", _wrap_new_float},
    { "delete_float", _wrap_delete_float},
    { "float_getitem", _wrap_float_getitem},
    { "float_setitem", _wrap_float_setitem},
    { "get_allegro_primitives_version", _wrap_get_allegro_primitives_version},
    { "init_primitives_addon", _wrap_init_primitives_addon},
    { "shutdown_primitives_addon", _wrap_shutdown_primitives_addon},
    { "draw_line", _wrap_draw_line},
    { "draw_triangle", _wrap_draw_triangle},
    { "draw_filled_triangle", _wrap_draw_filled_triangle},
    { "draw_rectangle", _wrap_draw_rectangle},
    { "draw_filled_rectangle", _wrap_draw_filled_rectangle},
    { "draw_rounded_rectangle", _wrap_draw_rounded_rectangle},
    { "draw_filled_rounded_rectangle", _wrap_draw_filled_rounded_rectangle},
    { "_calculate_arc", _wrap__calculate_arc},
    { "draw_pieslice", _wrap_draw_pieslice},
    { "draw_filled_pieslice", _wrap_draw_filled_pieslice},
    { "draw_ellipse", _wrap_draw_ellipse},
    { "draw_filled_ellipse", _wrap_draw_filled_ellipse},
    { "draw_circle", _wrap_draw_circle},
    { "draw_filled_circle", _wrap_draw_filled_circle},
    { "draw_arc", _wrap_draw_arc},
    { "draw_elliptical_arc", _wrap_draw_elliptical_arc},
    { "_calculate_spline", _wrap__calculate_spline},
    { "draw_spline", _wrap_draw_spline},
    { "_calculate_ribbon", _wrap__calculate_ribbon},
    { "_draw_ribbon", _wrap__draw_ribbon},
    { "draw_prim", _wrap_draw_prim},
    { "draw_indexed_prim", _wrap_draw_indexed_prim},
    { "draw_vertex_buffer", _wrap_draw_vertex_buffer},
    { "draw_indexed_buffer", _wrap_draw_indexed_buffer},
    { "create_vertex_decl", _wrap_create_vertex_decl},
    { "destroy_vertex_decl", _wrap_destroy_vertex_decl},
    { "create_vertex_buffer", _wrap_create_vertex_buffer},
    { "destroy_vertex_buffer", _wrap_destroy_vertex_buffer},
    { "lock_vertex_buffer", _wrap_lock_vertex_buffer},
    { "unlock_vertex_buffer", _wrap_unlock_vertex_buffer},
    { "get_vertex_buffer_size", _wrap_get_vertex_buffer_size},
    { "create_index_buffer", _wrap_create_index_buffer},
    { "destroy_index_buffer", _wrap_destroy_index_buffer},
    { "lock_index_buffer", _wrap_lock_index_buffer},
    { "unlock_index_buffer", _wrap_unlock_index_buffer},
    { "get_index_buffer_size", _wrap_get_index_buffer_size},
    { "_draw_polyline", _wrap__draw_polyline},
    { "_draw_polygon", _wrap__draw_polygon},
    { "_draw_filled_polygon", _wrap__draw_filled_polygon},
    { "_draw_filled_polygon_with_holes", _wrap__draw_filled_polygon_with_holes},
    {0,0}
};
static swig_lua_class* swig_SwigModule_classes[]= {
    0
};
static swig_lua_namespace* swig_SwigModule_namespaces[] = {
    0
};

static swig_lua_namespace swig_SwigModule = {
    "lallegro_primitives",
    swig_SwigModule_methods,
    swig_SwigModule_attributes,
    swig_SwigModule_constants,
    swig_SwigModule_classes,
    swig_SwigModule_namespaces
};
#ifdef __cplusplus
}
#endif

/* -------- TYPE CONVERSION AND EQUIVALENCE RULES (BEGIN) -------- */

static swig_type_info _swigt__p_ALLEGRO_BITMAP = {"_p_ALLEGRO_BITMAP", "ALLEGRO_BITMAP *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_ALLEGRO_COLOR = {"_p_ALLEGRO_COLOR", "ALLEGRO_COLOR *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_ALLEGRO_INDEX_BUFFER = {"_p_ALLEGRO_INDEX_BUFFER", "struct ALLEGRO_INDEX_BUFFER *|ALLEGRO_INDEX_BUFFER *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_ALLEGRO_LINE_CAP = {"_p_ALLEGRO_LINE_CAP", "enum ALLEGRO_LINE_CAP *|ALLEGRO_LINE_CAP *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_ALLEGRO_LINE_JOIN = {"_p_ALLEGRO_LINE_JOIN", "enum ALLEGRO_LINE_JOIN *|ALLEGRO_LINE_JOIN *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_ALLEGRO_PRIM_ATTR = {"_p_ALLEGRO_PRIM_ATTR", "enum ALLEGRO_PRIM_ATTR *|ALLEGRO_PRIM_ATTR *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_ALLEGRO_PRIM_BUFFER_FLAGS = {"_p_ALLEGRO_PRIM_BUFFER_FLAGS", "enum ALLEGRO_PRIM_BUFFER_FLAGS *|ALLEGRO_PRIM_BUFFER_FLAGS *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_ALLEGRO_PRIM_STORAGE = {"_p_ALLEGRO_PRIM_STORAGE", "enum ALLEGRO_PRIM_STORAGE *|ALLEGRO_PRIM_STORAGE *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_ALLEGRO_PRIM_TYPE = {"_p_ALLEGRO_PRIM_TYPE", "enum ALLEGRO_PRIM_TYPE *|ALLEGRO_PRIM_TYPE *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_ALLEGRO_VERTEX = {"_p_ALLEGRO_VERTEX", "struct ALLEGRO_VERTEX *|ALLEGRO_VERTEX *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_ALLEGRO_VERTEX_BUFFER = {"_p_ALLEGRO_VERTEX_BUFFER", "struct ALLEGRO_VERTEX_BUFFER *|ALLEGRO_VERTEX_BUFFER *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_ALLEGRO_VERTEX_DECL = {"_p_ALLEGRO_VERTEX_DECL", "struct ALLEGRO_VERTEX_DECL *|ALLEGRO_VERTEX_DECL *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_ALLEGRO_VERTEX_ELEMENT = {"_p_ALLEGRO_VERTEX_ELEMENT", "struct ALLEGRO_VERTEX_ELEMENT *|ALLEGRO_VERTEX_ELEMENT *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_float = {"_p_float", "float *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_int = {"_p_int", "int *", 0, 0, (void*)0, 0};
static swig_type_info _swigt__p_void = {"_p_void", "void *", 0, 0, (void*)0, 0};

static swig_type_info *swig_type_initial[] = {
  &_swigt__p_ALLEGRO_BITMAP,
  &_swigt__p_ALLEGRO_COLOR,
  &_swigt__p_ALLEGRO_INDEX_BUFFER,
  &_swigt__p_ALLEGRO_LINE_CAP,
  &_swigt__p_ALLEGRO_LINE_JOIN,
  &_swigt__p_ALLEGRO_PRIM_ATTR,
  &_swigt__p_ALLEGRO_PRIM_BUFFER_FLAGS,
  &_swigt__p_ALLEGRO_PRIM_STORAGE,
  &_swigt__p_ALLEGRO_PRIM_TYPE,
  &_swigt__p_ALLEGRO_VERTEX,
  &_swigt__p_ALLEGRO_VERTEX_BUFFER,
  &_swigt__p_ALLEGRO_VERTEX_DECL,
  &_swigt__p_ALLEGRO_VERTEX_ELEMENT,
  &_swigt__p_float,
  &_swigt__p_int,
  &_swigt__p_void,
};

static swig_cast_info _swigc__p_ALLEGRO_BITMAP[] = {  {&_swigt__p_ALLEGRO_BITMAP, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_ALLEGRO_COLOR[] = {  {&_swigt__p_ALLEGRO_COLOR, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_ALLEGRO_INDEX_BUFFER[] = {  {&_swigt__p_ALLEGRO_INDEX_BUFFER, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_ALLEGRO_LINE_CAP[] = {  {&_swigt__p_ALLEGRO_LINE_CAP, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_ALLEGRO_LINE_JOIN[] = {  {&_swigt__p_ALLEGRO_LINE_JOIN, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_ALLEGRO_PRIM_ATTR[] = {  {&_swigt__p_ALLEGRO_PRIM_ATTR, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_ALLEGRO_PRIM_BUFFER_FLAGS[] = {  {&_swigt__p_ALLEGRO_PRIM_BUFFER_FLAGS, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_ALLEGRO_PRIM_STORAGE[] = {  {&_swigt__p_ALLEGRO_PRIM_STORAGE, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_ALLEGRO_PRIM_TYPE[] = {  {&_swigt__p_ALLEGRO_PRIM_TYPE, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_ALLEGRO_VERTEX[] = {  {&_swigt__p_ALLEGRO_VERTEX, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_ALLEGRO_VERTEX_BUFFER[] = {  {&_swigt__p_ALLEGRO_VERTEX_BUFFER, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_ALLEGRO_VERTEX_DECL[] = {  {&_swigt__p_ALLEGRO_VERTEX_DECL, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_ALLEGRO_VERTEX_ELEMENT[] = {  {&_swigt__p_ALLEGRO_VERTEX_ELEMENT, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_float[] = {  {&_swigt__p_float, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_int[] = {  {&_swigt__p_int, 0, 0, 0},{0, 0, 0, 0}};
static swig_cast_info _swigc__p_void[] = {  {&_swigt__p_void, 0, 0, 0},{0, 0, 0, 0}};

static swig_cast_info *swig_cast_initial[] = {
  _swigc__p_ALLEGRO_BITMAP,
  _swigc__p_ALLEGRO_COLOR,
  _swigc__p_ALLEGRO_INDEX_BUFFER,
  _swigc__p_ALLEGRO_LINE_CAP,
  _swigc__p_ALLEGRO_LINE_JOIN,
  _swigc__p_ALLEGRO_PRIM_ATTR,
  _swigc__p_ALLEGRO_PRIM_BUFFER_FLAGS,
  _swigc__p_ALLEGRO_PRIM_STORAGE,
  _swigc__p_ALLEGRO_PRIM_TYPE,
  _swigc__p_ALLEGRO_VERTEX,
  _swigc__p_ALLEGRO_VERTEX_BUFFER,
  _swigc__p_ALLEGRO_VERTEX_DECL,
  _swigc__p_ALLEGRO_VERTEX_ELEMENT,
  _swigc__p_float,
  _swigc__p_int,
  _swigc__p_void,
};


/* -------- TYPE CONVERSION AND EQUIVALENCE RULES (END) -------- */

#include "SWIG_footer.inl"