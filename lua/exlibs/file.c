/**
 * file.c
 */

#include "lua.h"
#include "lauxlib.h"
#include "lrodefs.h"
#include "lexlibs.h"
#include "MicoPlatform.h"
#include "user_version.h"

#include <spiffs.h>
#include <spiffs_nucleus.h>
#define LOG_PAGE_SIZE       256
static u8_t spiffs_work_buf[LOG_PAGE_SIZE*2];
static u8_t spiffs_fds[32*4];
static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE+32)*4];
spiffs fs;
#define FILE_NOT_OPENED 0
static volatile int file_fd = FILE_NOT_OPENED;


#define MICO_FLASH_FOR_LUA       MICO_SPI_FLASH
//#define LUA_START_ADDRESS     (uint32_t)0x00100000 
//#define LUA_FLASH_SIZE        (uint32_t)1024*1024 
#define LUA_START_ADDRESS       (uint32_t)0x00C0000 
#define LUA_FLASH_SIZE          (uint32_t)1024*(1024+256) 

static s32_t lspiffs_read(u32_t addr, u32_t size, u8_t *dst) {
    MicoFlashRead(MICO_FLASH_FOR_LUA,&addr,dst,size);
    return SPIFFS_OK;
  }

static s32_t lspiffs_write(u32_t addr, u32_t size, u8_t *src) {
    MicoFlashWrite(MICO_FLASH_FOR_LUA,&addr,src,size);
    return SPIFFS_OK;
  }

static s32_t lspiffs_erase(u32_t addr, u32_t size) {
    MicoFlashErase(MICO_FLASH_FOR_LUA,addr,addr+size-1);
    MicoWdgReload();//in case wathdog
    return SPIFFS_OK;
  } 

void lua_spiffs_mount() {
    spiffs_config cfg;    
    cfg.phys_size = LUA_FLASH_SIZE;
    cfg.phys_addr = LUA_START_ADDRESS; // start spiffs at start of spi flash
    cfg.phys_erase_block = 65536/2; // according to datasheet
    cfg.log_block_size = 65536; // let us not complicate things
    cfg.log_page_size = LOG_PAGE_SIZE; // as we said */
    
    cfg.hal_read_f = lspiffs_read;
    cfg.hal_write_f = lspiffs_write;
    cfg.hal_erase_f = lspiffs_erase;
    
    MicoFlashInitialize(MICO_FLASH_FOR_LUA);
    
    if(SPIFFS_mounted(&fs)) return;
    
    int res = SPIFFS_mount(&fs,
      &cfg,
      spiffs_work_buf,
      spiffs_fds,
      sizeof(spiffs_fds),
      spiffs_cache_buf,
      sizeof(spiffs_cache_buf),
      0);
    
    //MCU_DBG("mount res: %i\r\n", res);
}

int mode2flag(char *mode){
  if(strlen(mode)==1){
  	if(strcmp(mode,"w")==0)
  	  return SPIFFS_WRONLY|SPIFFS_CREAT|SPIFFS_TRUNC;
  	else if(strcmp(mode, "r")==0)
  	  return SPIFFS_RDONLY;
  	else if(strcmp(mode, "a")==0)
  	  return SPIFFS_WRONLY|SPIFFS_CREAT|SPIFFS_APPEND;
  	else
  	  return SPIFFS_RDONLY;
  } else if (strlen(mode)==2){
  	if(strcmp(mode,"r+")==0)
  	  return SPIFFS_RDWR;
  	else if(strcmp(mode, "w+")==0)
  	  return SPIFFS_RDWR|SPIFFS_CREAT|SPIFFS_TRUNC;
  	else if(strcmp(mode, "a+")==0)
  	  return SPIFFS_RDWR|SPIFFS_CREAT|SPIFFS_APPEND;
  	else
  	  return SPIFFS_RDONLY;
  } else {
  	return SPIFFS_RDONLY;
  }
}

//table = file.list()
static int file_list( lua_State* L )
{
  spiffs_DIR d;
  struct spiffs_dirent e;
  struct spiffs_dirent *pe = &e;

  lua_newtable( L );
  SPIFFS_opendir(&fs, "/", &d);
  while ((pe = SPIFFS_readdir(&d, pe))) {
    lua_pushinteger(L, pe->size);
    lua_setfield( L, -2, (char*)pe->name );
    //MCU_DBG("  %s size:%i\r\n", pe->name, pe->size);
  }
  SPIFFS_closedir(&d);
  return 1;
}
//table = file.list()
static int file_slist( lua_State* L )
{
  spiffs_DIR d;
  struct spiffs_dirent e;
  struct spiffs_dirent *pe = &e;
  char buff[LUAL_BUFFERSIZE];
  SPIFFS_opendir(&fs, "/", &d);
  while ((pe = SPIFFS_readdir(&d, pe))) {
    sprintf(buff," %s size:%i\r\n", pe->name, pe->size);
    l_message(NULL,buff);
  }
  SPIFFS_closedir(&d);
  return 1;
}
//file.format()
static int file_format( lua_State* L )
{
  if(SPIFFS_mounted(&fs)==false) lua_spiffs_mount();
   
  SPIFFS_unmount(&fs);
  
  int ret = SPIFFS_format(&fs);
  if(ret==SPIFFS_OK)
  {
    MCU_DBG("format done\r\n");
    lua_spiffs_mount();    
  }
  else
    MCU_DBG("format error,err:%d\r\n",ret);
  return 0;
}

// file.open(filename, mode)
static int file_open( lua_State* L )
{
  size_t len;
  const char *fname = luaL_checklstring( L, 1, &len );
  if( len > SPIFFS_OBJ_NAME_LEN )
    return luaL_error(L, "filename too long");
  
  if(FILE_NOT_OPENED!=file_fd){
    SPIFFS_close(&fs,file_fd);
    file_fd = FILE_NOT_OPENED;
  }
  const char *mode = luaL_optstring(L, 2, "r");
  file_fd = SPIFFS_open(&fs,(char*)fname,mode2flag((char*)mode),0);
  if(file_fd < FILE_NOT_OPENED){
    file_fd = FILE_NOT_OPENED;
    lua_pushnil(L);
  } else {
    lua_pushboolean(L, true);
  }
  return 1; 
}

// file.close()
static int file_close( lua_State* L )
{
  if(FILE_NOT_OPENED!=file_fd){
    SPIFFS_close(&fs,file_fd);
    file_fd = FILE_NOT_OPENED;
  }
  return 0;  
}

// file.write("string")
static int file_write( lua_State* L )
{
  if(FILE_NOT_OPENED==file_fd)
    return luaL_error(L, "open a file first");
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  if(SPIFFS_write(&fs,file_fd, (char*)s, len)<0)
  {//failed
    SPIFFS_close(&fs,file_fd);
    file_fd = FILE_NOT_OPENED;
    lua_pushnil(L);
  }
  else//success
    lua_pushboolean(L, true);
  return 1;
}
// file.writeline("string")
static int file_writeline( lua_State* L )
{
  if(FILE_NOT_OPENED==file_fd)
    return luaL_error(L, "open a file first");
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  if(SPIFFS_write(&fs,file_fd, (char*)s, len)<0)
  {//failed
    lua_pushnil(L);
    SPIFFS_close(&fs,file_fd);
    file_fd = FILE_NOT_OPENED;
  }
  else
  {//success
     if(SPIFFS_write(&fs,file_fd, "\r\n", 2)<0)
     {
        SPIFFS_close(&fs,file_fd);
        file_fd = FILE_NOT_OPENED;
        lua_pushnil(L);
     }
     else
        lua_pushboolean(L, true);
  }
  return 1;
}

static int file_g_read( lua_State* L, int n, int16_t end_char )
{
  if(n< 0 || n>LUAL_BUFFERSIZE) 
    n = LUAL_BUFFERSIZE;
  if(end_char < 0 || end_char >255)
    end_char = EOF;
  int ec = (int)end_char;
  
  luaL_Buffer b;
  if(FILE_NOT_OPENED==file_fd)
    return luaL_error(L, "open a file first");

  luaL_buffinit(L, &b);
  char *p = luaL_prepbuffer(&b);
  int c = EOF;
  int i = 0;

  do{
    c = EOF;
    SPIFFS_read(&fs, (spiffs_file)file_fd, &c, 1);
    if(c==EOF)break;
    p[i++] = (char)(0xFF & c);
  }while((c!=EOF) && ((char)c !=(char)ec) && (i<n) );

#if 0
  if(i>0 && p[i-1] == '\n')
    i--;    /* do not include `eol' */
#endif
    
  if(i==0){
    luaL_pushresult(&b);  /* close buffer */
    return (lua_objlen(L, -1) > 0);  /* check whether read something */
  }

  luaL_addsize(&b, i);
  luaL_pushresult(&b);  /* close buffer */
  return 1;  /* read at least an `eol' */ 
}

// file.read()
// file.read() read all byte in file LUAL_BUFFERSIZE(512) max
// file.read(10) will read 10 byte from file, or EOF is reached.
// file.read('q') will read until 'q' or EOF is reached. 
static int file_read( lua_State* L )
{
  unsigned need_len = LUAL_BUFFERSIZE;
  int16_t end_char = EOF;
  size_t el;
  if( lua_type( L, 1 ) == LUA_TNUMBER )
  {
    need_len = ( unsigned )luaL_checkinteger( L, 1 );
    if( need_len > LUAL_BUFFERSIZE ){
      need_len = LUAL_BUFFERSIZE;
    }
  }
  else if(lua_isstring(L, 1))
  {
    const char *end = luaL_checklstring( L, 1, &el );
    if(el!=1){
      return luaL_error( L, "wrong arg range" );
    }
    end_char = (int16_t)end[0];
  }
  return file_g_read(L, need_len, end_char);
}
// file.readline()
static int file_readline( lua_State* L )
{
  return file_g_read(L, LUAL_BUFFERSIZE, '\n');
}

//file.seek(whence, offset)
static int file_seek (lua_State *L) 
{
  static const int mode[] = {SPIFFS_SEEK_SET, SPIFFS_SEEK_CUR, SPIFFS_SEEK_END};
  static const char *const modenames[] = {"set", "cur", "end", NULL};
  if(FILE_NOT_OPENED==file_fd)
    return luaL_error(L, "open a file first");
  int op = luaL_checkoption(L, 1, "cur", modenames);
  long offset = luaL_optlong(L, 2, 0);
  op = SPIFFS_lseek(&fs,file_fd, offset, mode[op]);
  if (op)
    lua_pushnil(L);  /* error */
  else
  {
    spiffs_fd *fd;
    spiffs_fd_get(&fs, file_fd, &fd);
    lua_pushinteger(L, fd->fdoffset);
  }
  return 1;
}

// file.flush()
static int file_flush( lua_State* L )
{
  if(FILE_NOT_OPENED==file_fd)
    return luaL_error(L, "open a file first");
  if(SPIFFS_fflush(&fs,file_fd) == 0)
    lua_pushboolean(L, 1);
  else
    lua_pushnil(L);
  return 1;
}
// file.remove(filename)
static int file_remove( lua_State* L )
{
  size_t len;
  const char *fname = luaL_checklstring( L, 1, &len );
  if( len > SPIFFS_OBJ_NAME_LEN )
    return luaL_error(L, "filename too long");
  file_close(L);
  SPIFFS_remove(&fs, (char *)fname);
  return 0;  
}

// file.rename("oldname", "newname")
static int file_rename( lua_State* L )
{
  size_t len;

  if(FILE_NOT_OPENED!=file_fd){
    SPIFFS_close(&fs,file_fd);
    file_fd = FILE_NOT_OPENED;
  }

  const char *oldname = luaL_checklstring( L, 1, &len );
  if( len > SPIFFS_OBJ_NAME_LEN )
    return luaL_error(L, "filename too long");

  const char *newname = luaL_checklstring( L, 2, &len );
  if( len > SPIFFS_OBJ_NAME_LEN )
    return luaL_error(L, "filename too long");

  if(SPIFFS_OK==SPIFFS_rename(&fs, (char*)oldname, (char*)newname )){
    lua_pushboolean(L, 1);
  } else {
    lua_pushboolean(L, 0);
  }
  return 1;
}
//file.info()
static int file_info( lua_State* L )
{
  uint32_t total, used;
  SPIFFS_info(&fs, &total, &used);
  if(total>0x7FFFFFFF || used>0x7FFFFFFF || used > total)
  {
    return luaL_error(L, "file system error");;
  }
  lua_pushinteger(L, total-used);
  lua_pushinteger(L, used);
  lua_pushinteger(L, total);
  return 3;
}
//file.state()
static int file_state( lua_State* L )
{
  if(FILE_NOT_OPENED==file_fd)
  return luaL_error(L, "open a file first");
  
  spiffs_stat s;
  SPIFFS_fstat(&fs, file_fd, &s);
  
  lua_pushstring(L,(char*)s.name);
  lua_pushinteger(L,s.size);
  return 2;
}

#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"
static int writer(lua_State* L, const void* p, size_t size, void* u)
{
  UNUSED(L);
  int file_fd = *( (int *)u );
  if ( FILE_NOT_OPENED == file_fd)
    return 1;
  //MCU_DBG("get fd:%d,size:%d\n", file_fd, size);

  if (size != 0 && (size != SPIFFS_write(&fs,file_fd, (char *)p, size)) )
    return 1;
  //MCU_DBG("write fd:%d,size:%d\n", file_fd, size);
  return 0;
}
#define toproto(L,i) (clvalue(L->top+(i))->l.p)
static int file_compile( lua_State* L )
{
  Proto* f;
  int file_fd = FILE_NOT_OPENED;
  size_t len;
  const char *fname = luaL_checklstring( L, 1, &len );
  if ( len > SPIFFS_OBJ_NAME_LEN )
    return luaL_error(L, "filename too long");

  char output[SPIFFS_OBJ_NAME_LEN];
  strcpy(output, fname);
  // check here that filename end with ".lua".
  if (len < 4 || (strcmp( output + len - 4, ".lua") != 0) )
    return luaL_error(L, "not a .lua file");

  output[strlen(output) - 2] = 'c';
  output[strlen(output) - 1] = '\0';
  
  if (luaL_loadfile(L, fname) != 0) {
    return luaL_error(L, lua_tostring(L, -1));
  }

  f = toproto(L, -1);

  int stripping = 1;      /* strip debug information? */

  file_fd = SPIFFS_open(&fs,(char*)output,mode2flag("w+"),0);
  if (file_fd < FILE_NOT_OPENED)
  {
    return luaL_error(L, "cannot open/write to file");
  }

  lua_lock(L);
  int result = luaU_dump(L, f, writer, &file_fd, stripping);
  lua_unlock(L);

  SPIFFS_fflush(&fs,file_fd);
  SPIFFS_close(&fs,file_fd);
  file_fd =FILE_NOT_OPENED;

  if (result == LUA_ERR_CC_INTOVERFLOW) {
    return luaL_error(L, "value too big or small for target integer type");
  }
  if (result == LUA_ERR_CC_NOTINTEGER) {
    return luaL_error(L, "target lua_Number is integral but fractional value found");
  }

  return 0;
}


const LUA_REG_TYPE file_map[] =
{
  { LSTRKEY( "list" ), LFUNCVAL( file_list ) },
  { LSTRKEY( "slist" ), LFUNCVAL( file_slist ) },
  { LSTRKEY( "format" ), LFUNCVAL( file_format ) },
  { LSTRKEY( "open" ), LFUNCVAL( file_open ) },
  { LSTRKEY( "close" ), LFUNCVAL( file_close ) },
  { LSTRKEY( "write" ), LFUNCVAL( file_write ) },
  { LSTRKEY( "writeline" ), LFUNCVAL( file_writeline ) },
  { LSTRKEY( "read" ), LFUNCVAL( file_read ) },
  { LSTRKEY( "readline" ), LFUNCVAL( file_readline ) },
  { LSTRKEY( "remove" ), LFUNCVAL( file_remove ) },
  { LSTRKEY( "seek" ), LFUNCVAL( file_seek ) },
  { LSTRKEY( "flush" ), LFUNCVAL( file_flush ) },
  { LSTRKEY( "rename" ), LFUNCVAL( file_rename ) },
  { LSTRKEY( "info" ), LFUNCVAL( file_info ) },
  { LSTRKEY( "state" ), LFUNCVAL( file_state ) },
  { LSTRKEY( "compile" ), LFUNCVAL( file_compile ) },
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_file(lua_State *L)
{
  lua_spiffs_mount();

  luaL_register( L, EXLIB_FILE, file_map );
  return 1;
}
