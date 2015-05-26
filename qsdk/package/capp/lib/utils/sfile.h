#ifndef __SFILE_H_CAF3737A24BFD00978F62CEB481C30B0__
#define __SFILE_H_CAF3737A24BFD00978F62CEB481C30B0__
#ifdef __APP__
/******************************************************************************/
/* simpile file api */
/******************************************************************************/
#ifndef os_fopen
#define os_fopen(_file, _mode)  ({  \
    FILE *__stream = fopen(_file, _mode); \
    if (NULL==__stream) {           \
        debug_error("open %s error:%d", _file, errno); \
    }                               \
    __stream;                       \
})  /* end */
#endif

#ifndef __os_fclose
#define __os_fclose(_stream)    ({  \
    int __err = 0;                  \
    FILE *___stream = (_stream);    \
    if (___stream) {                \
        __err = fclose(___stream);  \
    }                               \
    __err;                          \
})  /* end */
#endif

#ifndef os_fclose
#define os_fclose(_stream)      ({  \
    int __err=__os_fclose(_stream); \
    _stream = NULL;                 \
    __err;                          \
})  /* end */
#endif

#ifndef os_fclean
#define os_fclean(_file)        __os_fclose(fopen(_file, "w+"))
#endif

#ifndef os_feof
#define os_feof(_stream)            ({  \
    FILE *___stream = (_stream);        \
    ___stream?(!!feof(___stream)):true; \
})  /* end */
#endif

#ifndef os_fwrite
#define os_fwrite(_stream, _buf, _size)         ({  \
    FILE *___stream = (_stream);                    \
    int __err = fwrite(_buf, _size, 1, ___stream);  \
    __err = __err<0?(-errno):0;                     \
    fflush(___stream);                              \
    __err;                                          \
})  /* end */
#endif

#ifndef os_fread
#define os_fread(_stream, _buf, _size)          ({  \
    FILE *___stream = (_stream);                    \
    int __err = fread(_buf, _size, 1, ___stream);   \
    __err = __err<0?(-errno):0;                     \
    __err;                                          \
})  /* end */
#endif

#ifndef os_freadline
#define os_freadline(_stream, _line, _size)     ({  \
    FILE *___stream = (_stream);                    \
    char *__line = (char *)(_line);                 \
    int __size = (int)(_size);                      \
    char *__p = NULL;                               \
                                                    \
    if (___stream && __line && __size > 0) {        \
        __p = fgets(__line, __size, ___stream);     \
    }                                               \
                                                    \
    __p;                                            \
})  /* end */
#endif

#ifndef os_fwriteline
#define os_fwriteline(_stream, _line)           ({  \
    FILE *___stream = (_stream);                    \
    char *__line = (char *)(_line);                 \
    char *__p = NULL;                               \
                                                    \
    if (___stream && __line) {                      \
        __p = fputs(__line, ___stream);             \
        fputc('\n', ___stream);                     \
    }                                               \
                                                    \
    __p;                                            \
})  /* end */
#endif

static inline bool 
os_sfscan_match_all(char *context)
{   /*
    * just "*"
    */
    return '*'==context[0] && 0==context[1];
}

static inline bool
os_sfcan_is_dir_self(char *filename/* not include path */)
{
    return '.'==filename[0] && 0==filename[1];
}

static inline bool
os_sfcan_is_dir_father(char *filename/* not include path */)
{
    return '.'==filename[0] && '.'==filename[1] && 0==filename[2];
}

/*
* @filename: not include path
*/
typedef multi_value_t os_sfscan_line_handle_t(char *filename, char *line, void *control);
typedef multi_value_t os_sfscan_file_handle_t(char *path, char *filename, 
                            os_sfscan_line_handle_t *line_handle, void *control);
typedef bool os_sfscan_file_filter_t(char *path, char *filename, void *control);

static inline multi_value_t 
os_sfscan_file_handle
(
    char *path,
    char *filename,
    os_sfscan_line_handle_t *line_handle,
    void *control
)
{
    FILE *stream = NULL;
    char line[1+OS_LINE_LEN];
    multi_value_u mv;

    if (NULL==path) {
        return os_assertV(mv2_GO(-EINVAL1));
    }
    else if (NULL==filename) {
        return os_assertV(mv2_GO(-EINVAL2));
    }
    else if (NULL==line_handle) {
        return os_assertV(mv2_GO(-EINVAL3));
    }
    
    stream = os_v_fopen("r", "%s/%s", path, filename);
    if (NULL==stream) {
        debug_error("open %s/%s error:%d", path, filename, -errno);
        
        mv2_result(mv) = -errno;
        
        goto error;
    }
    
    while(!os_feof(stream)) {
        line[0] = 0;
        os_freadline(stream, line, OS_LINE_LEN);
        
        /*
        * strim left/right blank
        */
        __string_strim_both(line, NULL);
        
        /*
        * replace blank("\t \r\n") to ' '
        */
        __string_replace(line, NULL, ' ');
        
        /*
        * reduce ' '
        */
        __string_reduce(line, NULL);
        
        /*
        * skip blank line
        */
        if (__is_blank_line(line)) {
            continue;
        }
        
        /*
        * skip notes line
        */
        if (__is_notes_line_deft(line)) {
            continue;
        }
        
        mv.value = (*line_handle)(filename, line, control);
        if (mv2_is_break(mv)) {
            goto error;
        }
    }

    mv.value = mv2_OK;
error:
    os_fclose(stream);
    
    return mv.value;
}

/*
* @filefilter: if return true, ignore file
*/
static inline int 
__os_sfscan_dir
(
    char *path, 
    bool recur,
    os_sfscan_file_filter_t *file_filter,
    os_sfscan_file_handle_t *file_handle,
    os_sfscan_line_handle_t *line_handle,
    void *control
)
{
    DIR *dir = NULL;
    struct dirent *file = NULL;
    struct stat st;
    multi_value_u mv;
    int err = 0;

    if (NULL==path) {
        return os_assertV(-EINVAL1);
    }
    else if (NULL==file_handle) {
        return os_assertV(-EINVAL2);
    }

    debug_trace("begin scan %s", path);
    
    dir = opendir(path);
    if (NULL == dir) {
        err = -errno;

        debug_error("open dir %s error:%d", path, errno);
        
        goto error;
    }
    
    while (NULL != (file=readdir(dir))) {
        char *filename = file->d_name; /* just name, not include path */

        debug_trace("scan %s/%s", path, filename);

        /*
        * skip . and ..
        */
        if (os_sfcan_is_dir_self(filename) || os_sfcan_is_dir_father(filename)) {
            debug_trace("skip %s/%s", path, filename);
            
            continue;
        }
        
        /*
        * dir
        */
        if (stat(filename, &st) >= 0 && S_ISDIR(st.st_mode)) {
            if (recur) {
                char line[1+OS_LINE_LEN];

                os_sprintf(line, "%s/%s", path, filename);
                
                err = __os_sfscan_dir(line, recur, file_filter, file_handle, line_handle, control);
                if (err) {
                    goto error;
                }
            } else {
                continue;
            }
        }
        
        /*
        * file filter
        */
        if (file_filter && (*file_filter)(path, filename, control)) {
            debug_trace("skip %s/%s", path, filename);
            
            continue;
        }
        
        /*
        * file handle
        */
        mv.value = (*file_handle)(path, filename, line_handle, control);
        if (mv2_is_break(mv)) {
            err = mv2_result(mv);

            debug_error("handle %s/%s error:%d", path, filename, err);
            
            goto error;
        }
    }

    debug_trace("end scan %s error:%d", path, err);
error:
    if (dir) {
        closedir(dir);
    }

    return err;
}

static inline int 
os_sfscan_dir
(
    char *path, 
    bool recur,
    os_sfscan_file_filter_t *file_filter,
    os_sfscan_line_handle_t *line_handle,
    void *control
)
{
    return __os_sfscan_dir(path, recur, file_filter, os_sfscan_file_handle, line_handle, control);
}
/******************************************************************************/
#define __os_sgetx(_prefix, _stream, _vfmt, _pv) ({ \
    int __err = 0;                  \
                                    \
    if (NULL==(_stream)) {          \
        __err = -errno;             \
    } else if (1!=fscanf(_stream, _vfmt, _pv)) {  \
        __err = -EFORMAT;           \
    }                               \
                                    \
    if (_stream) {                  \
        __err = __os_wait_error(_prefix##close(_stream)); \
    }                               \
                                    \
    __err;                          \
})

#define __os_sgets(_prefix, _stream, _line, _space) ({ \
    int __err = 0;                  \
                                    \
    if (NULL==(_line)) {            \
        __err = -EINVAL9;           \
    } else if ((_space)<=0) {       \
        __err = -EINVAL8;           \
    } else if (NULL==_stream) {     \
        __err = -EINVAL7;           \
    } else if (NULL==os_freadline(_stream, _line, _space)) { \
        __err = -errno;             \
    } else {                        \
        __string_strim_both(_line, NULL); \
    }                               \
                                    \
    if (_stream) {                  \
        __err = __os_wait_error(_prefix##close(_stream)); \
    }                               \
                                    \
    __err;                          \
}) /* end */

#define os_sgetx(_prefix, _vfmt, _pv, _filename)        ({  \
    FILE *__stream = _prefix##open(_filename, "r");         \
    int __err = __os_sgetx(_prefix, __stream, _vfmt, _pv);  \
    __err; \
})  /* end */

#define os_sgets(_prefix, _line, _space, _filename)     ({  \
    FILE *__stream = _prefix##open(_filename, "r");         \
    int __err = __os_sgets(_prefix, __stream, _line, _space); \
    __err;                                                  \
})  /* end */

#define os_sgeti(_prefix, _pi, _filename)    \
    os_sgetx(_prefix, "%u", _pi, _filename)
#define os_sgetll(_prefix, _pll, _filename)  \
    os_sgetx(_prefix, "%llu", _pll, _filename)

#define os_v_sgetx(_prefix, _vfmt, _pv, _fmt, _args...)     ({  \
    FILE *__stream = os_v_##_prefix##open("r", _fmt, ##_args);  \
    int __err = __os_sgetx(_prefix, __stream, _vfmt, _pv);      \
    __err;                                                      \
})  /* end */

#define os_v_sgets(_prefix, _line, _space, _fmt, _args...)  ({  \
    FILE *__stream = os_v_##_prefix##open("r", _fmt, ##_args);  \
    int __err = __os_sgets(_prefix, __stream, _line, _space);   \
    __err;                                                      \
})  /* end */

#define os_v_sgeti(_prefix, _pi, _fmt, _args...)     \
    os_v_sgetx(_prefix, "%u", _pi, _fmt, ##_args)
#define os_v_sgetll(_prefix, _pll, _fmt, _args...)   \
    os_v_sgetx(_prefix, "%u", _pll, _fmt, ##_args)

/*
* get (_string/int/long long int) from file
*/
#define os_sfgets(_line, _space, _filename)         os_sgets(f, _line, _space, _filename)
#define os_sfgeti(_pi, _filename)                   os_sgeti(f, _pi, _filename)
#define os_sfgetll(_pll, _filename)                 os_sgetll(f, _pll, _filename)

#define os_v_sfgets(_line, _space, _fmt, _args...)  os_v_sgets(f, _line, _space, _fmt, ##_args)
#define os_v_sfgeti(_pi, _fmt, _args...)            os_v_sgeti(f, _pi, _fmt, ##_args)
#define os_v_sfgetll(_pll, _fmt, _args...)          os_v_sgeti(f, _pll, _fmt, ##_args)

/*
* get (_string/int/long long int) from pipe
*/
#define os_spgets(_line, _space, _filename)         os_sgets(p, _line, _space, _filename)
#define os_spgeti(_pi, _filename)                   os_sgeti(p, _pi, _filename)
#define os_spgetll(_pll, _filename)                 os_sgetll(p, _pll, _filename)

#define os_v_spgets(_line, _space, _fmt, _args...)  os_v_sgets(p, _line, _space, _fmt, ##_args)
#define os_v_spgeti(_pi, _fmt, _args...)            os_v_sgeti(p, _pi, _fmt, ##_args)
#define os_v_spgetll(_pll, _fmt, _args...)          os_v_sgeti(p, _pll, _fmt, ##_args)

#define __os_ssetx(_prefix, _stream, _vfmt, v) ({ \
    int __err = 0;              \
    FILE *__stream = (_stream); \
                                \
    if (NULL==(__stream)) {     \
        __err = -errno;         \
    } else {                    \
        __err = fprintf(__stream, _vfmt, v);      \
    }                           \
                                \
    if (__stream) {             \
        __err = __os_wait_error(_prefix##close(__stream)); \
    }                           \
                                \
    __err;                      \
})  /* end */

#define os_ssetx(_prefix, _vfmt, _pv, _filename)        ({  \
    FILE *__stream = _prefix##open(_filename, "w");         \
    int __err = __os_ssetx(_prefix, __stream, _vfmt, _pv);  \
    __err;                                                  \
})  /* end */

#define os_v_ssetx(_prefix, _vfmt, _pv, _fmt, _args...)     ({  \
    FILE *__stream = os_v_##_prefix##open("w", _fmt, ##_args);  \
    int __err = __os_ssetx(_prefix, __stream, _vfmt, _pv);      \
    __err;                                                      \
})  /* end */
    
#define os_ssets(_prefix, _string, _filename)          os_ssetx(_prefix, "%s", _string, _filename)
#define os_sseti(_prefix, _vi, _filename)              os_ssetx(_prefix, "%u", _vi, _filename)
#define os_ssetll(_prefix, _vll, _filename)            os_ssetx(_prefix, "%llu", _vll, _filename)

#define os_v_ssets(_prefix, _string, _fmt, _args...)    os_v_ssetx(_prefix, "%s", _string, _fmt, ##_args)
#define os_v_sseti(_prefix, _vi, _fmt, _args...)        os_v_ssetx(_prefix, "%u", _vi, _fmt, ##_args)
#define os_v_ssetll(_prefix, _vll, _fmt, _args...)      os_v_ssetx(_prefix, "%llu", _vll, _fmt, ##_args)

/*
* set (_string/int/long long int) to file
*/
#define os_sfsets(_string, _filename)                 os_ssets(f, _string, _filename)
#define os_sfseti(_vi, _filename)                     os_sseti(f, _vi, _filename)
#define os_sfsetll(_vll, _filename)                   os_ssetll(f, _vll, _filename)

#define os_v_sfsets(_string, _fmt, _args...)           os_v_ssets(f, _string, _fmt, ##_args)
#define os_v_sfseti(_vi, _fmt, _args...)               os_v_sseti(f, _vi, _fmt, ##_args)
#define os_v_sfsetll(_vll, _fmt, _args...)             os_v_ssetll(f, _vll, _fmt, ##_args)

/*
* get file size by full filename(include path)
*/
static inline int
os_sfsize(char *filename)
{
    struct stat st;
    int err;
    
    err = stat(filename, &st);
    if (err) {
        return -errno;
    } else {
        return st.st_size;
    }
}

/*
* get file size
*/
#define os_v_sfsize(_fmt, _args...) ({  \
    int __size;                         \
    char __buf[1+OS_LINE_LEN] = {0};    \
                                        \
    os_saprintf(__buf, _fmt, ##_args);  \
    __size = os_sfsize(__buf);          \
                                        \
    __size;                             \
})

static inline int
__os_lock_file(char *file, int open_mode, int permit, bool block)
{
    int err, fd;
    int deft = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    
    fd = open(file, open_mode, permit?permit:deft);
        debug_trace_error((fd<0?errno:0), "open %s", file);
    if (fd<0) {
        return -errno;
    }
    
    err = flock(fd, LOCK_EX | (block?0:LOCK_NB));
        debug_trace_error((err?errno:0), "lock %s", file);
    if (err) {
        return -errno;
    }
    
    return fd;
}

static int
os_lock_file(char *file)
{
    int fd = __os_lock_file(file, 
        (O_RDONLY | O_CREAT), (S_IRUSR | S_IRGRP), false);
    if (is_good_fd(fd)) {
        return 0;
    } else {
        return fd;
    }
}

static int
__os_unlock_file(char *file, int fd)
{
    int err;

    if (false==is_good_fd(fd)) {
        return -ENOLOCK;
    }
    
    err = flock(fd, LOCK_UN);
        debug_trace_error((err?errno:0), "unlock %s", file);
    if (err) {
        /* NOT return */
    }

    err = close(fd);
        debug_trace_error((err?errno:0), "close %s", file);
    if (err) {
        return err;
    }
    
    return 0;
}

#define os_unlock_file(_file, _fd)      ({  \
    int __err = __os_unlock_file(_file, _fd); \
    _fd = INVALID_FD;                       \
    __err;                                  \
})  /* end */

static bool
os_file_exist(char *file)
{
    int fd = open(file, O_RDONLY, S_IRUSR | S_IRGRP);
    if(fd<0) {
        return false;
    } else {
        close(fd);

        return true;
    }
}

#ifndef SCRIPT_SHELL_SYSTEM
#define SCRIPT_SHELL_SYSTEM     "/usr/bin/csystem"
#endif

#define os_p_system(_fmt, _args...) ({  \
    int err = 0, code = 0;          \
                                    \
    if (os_file_exist(SCRIPT_SHELL_SYSTEM)) { \
        err = os_v_spgeti(&code, SCRIPT_SHELL_SYSTEM " " _fmt, ##_args); \
        if (0==err) {               \
            err = code;             \
        }                           \
    } else {                        \
        err = os_system(_fmt, ##_args); \
    }                               \
                                    \
    err;                            \
})  /* end */

/******************************************************************************/
#endif /* __APP__ */
#endif /* __SFILE_H_CAF3737A24BFD00978F62CEB481C30B0__ */
