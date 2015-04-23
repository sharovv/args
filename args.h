#ifndef _args_h
#define _args_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* List of options format, must be "args_list" named.
   sequense of special characters &$#%^, every is followed with string
   - short form literal, one character ARGS_COPT
   = long form name string             ARGS_SOPT
   # value name, if option has a value ARGS_VAL
   % text with comment for this option ARGS_COMMENT
   ^ value default, if value # present ARGS_DEFAULT
   Ends with the NULL item.
   For example:
   "-w=warnings-level#LEVEL%Warnings level^3"
 */

/* Check for errors in command line
   return 1 if succeed, or 0 if error
   argc and argv are used from given in main() call
   list contains all valid options and their descriptions in format listed above
   error will point to error's description when 0 returned
 */
int args_check();

/* Get string with list of options, formatted for given width (columns) */
char *args_help();

/* Get first argument of command line
   Example:
     -v -w 5 file1.ext file2.ext
   return file1.ext
 */
char *args_arg();

/* Get n-th argument of command line, count from zero
   Example:
     --alpha 5 source.file --betta 6 destination.file
   with n=0 returns "source.file"
        n=1 returns "destination.file"
 */
char *args_argn( const int n );

/* Get option  with no value from command line
   Examples of short form:
   -v
   -abv
   Examples of long form:
   --verbose
   return 1 if option present, or 0 if option not found.
 */
int args_opt( const char *opt );

/* Get option's value from command line
   Examples of short form:
   -w5
   -w 5
   -w=5
   Examples of long form:
   --warnings-level=5
   --warnings-level 5
   return "5"
*/
char *args_val( const char *opt );

/* Get n-th option's value from command line
   Examples of short form:
   -oremount -o rw -o=bind
   Examplse of long form:
   --option remount --option=rw
*/
char *args_valn( const char *opt, const int n );

enum { ARGS_NONE, ARGS_SKIP, ARGS_ARG, ARGS_DASH, ARGS_DASH2, ARGS_COPT, ARGS_SOPT, ARGS_EQUAL, ARGS_VALUE, ARGS_VALUE_EQ, ARGS_VALUE_SKIP, ARGS_VALUE_SKIP_EQ, ARGS_END };

static int args_error_copt = 0;
static const char *args_error_sopt = 0;

/* Compare given option with item from list */
static int args_eq( const char *opt, const char *list )
{
  const char *s, *s2;

  /* list item must have option type - short or long */
  if( (s = strchr( list, opt[0] )) == NULL )
  {
    return 0;
  }
  /* first character: '-' for short form, '$' for long form */
  switch( opt[0] )
  {
  case '-':
    // simple compare one character
    return (s[1] == opt[1]);
  case '=':
    // end of long option string
    if( (s2 = strchr( s + 1, '#' )) == NULL )
    {
      return 0;
    }
    // length must be same
    if( strlen( opt + 1 ) != (size_t)(s2 - s - 1) )
    {
      return 0;
    }
    // and string must be equal
    return (strncmp( opt + 1, s + 1, s2 - s - 1 ) == 0);
  default:
    break;
  }
  // other differs
  return 0;
}

static char *args_parse( int argc, char *argv[], char *list[], const int nlist, const char *opt, const int n, const int need_value )
{
  int c, na = 1,nc = 0, stage = ARGS_NONE, narg = 0, nopt = 0, i;
  char *s, *s1, *s2, *sq;

  // till the end of argv
  while( stage != ARGS_END && na < argc )
  {
    // current literal
    c = argv[na][nc];
    // proceed different stages
    switch( stage )
    {
    case ARGS_NONE:
      /* first dash means short or long option */
      if( c == '-' )
      {
        stage = ARGS_DASH;
      }
      else
      {
        /* if no option required */
        if( opt == NULL )
        {
          /* and that argument */
          if( narg == n )
          {
            return argv[na];
          }
        }
        /* next argument */
        narg++;
        /* skip until line ends */
        stage = ARGS_SKIP;
      }
      break;
    case ARGS_DASH:
      /* second dash means long option */
      if( c == '-' )
      {
        stage = ARGS_SOPT;
        break;
      }
      stage = ARGS_COPT;
    case ARGS_COPT:
      /* is option in the list? */
      for( i = 0, s = NULL; i < nlist; i++ )
      {
        if( list[i][0] != '-' )
        {
          continue;
        }
        if( list[i][1] == c )
        {
          s = list[i];
          break;
        }
      }
      /* wrong option - error */
      if( s == NULL )
      {
        args_error_copt = c;
        stage = ARGS_END;
        return NULL;
      }
      /* need option to return? */
      if( opt != NULL )
      {
        /* this option is what we are looking for? */
        if( args_eq( opt, s ) )
        {
          /* and their index too */
          if( nopt == n )
          {
            /* if no value needed */
            if( !need_value )
            {
              /* just return non-NULL pointer */
              return argv[na] + nc;
            }
            /* ready to get value */
            stage = ARGS_VALUE;
          }
          /* next option's index */
          nopt++;
          break;
        }
      }
      if( strchr( s, '#' ) != NULL )
      {
        stage = ARGS_VALUE_SKIP;
      }
      break;
    case ARGS_VALUE:
      /* equal found, value follow */
      if( c == '=' )
      {
        stage = ARGS_VALUE_EQ;
        break;
      }
    case ARGS_VALUE_EQ:
      return argv[na] + nc;
    case ARGS_SOPT:
      /* scan list for long option */
      for( i = 0, s = NULL; i < nlist; i++ )
      {
        if( (s1 = strchr( list[i], '=' )) == NULL )
        {
          continue;
        }
        if( (s2 = strchr( s1 + 1, '#' )) == NULL )
        {
          if( (s2 = strchr( s1 + 1, '%' )) == NULL )
          {
            s2 = s1 + strlen( s1 );
          }
        }
        if( (sq = strchr( argv[na] + nc, '=' )) != NULL )
        {
          if( (sq - argv[na] - nc) != (s2 - s1 - 1) )
          {
            continue;
          }
        }
        if( strncmp( s1 + 1, argv[na] + nc, s2 - s1 - 1 ) == 0 )
        {
          s = list[i];
          break;
        }
      }
      /* wrong option - error */
      if( s == NULL )
      {
        args_error_sopt = argv[na] + nc;
        stage = ARGS_END;
        return NULL;
      }
      /* skip option's name */
      nc += (int)(s2 - s1 - 2);
      /* option needed? */
      if( opt != NULL )
      {
        if( args_eq( opt, s ) )
        {
          if( nopt == n )
          {
            if( !need_value )
            {
              return argv[na] + nc;
            }
            stage = ARGS_VALUE;
          }
          nopt++;
          break;
        }
      }
      if( strchr( s, '#' ) != NULL )
      {
        stage = ARGS_VALUE_SKIP;
      }
      else
      {
        stage = ARGS_SKIP;
      }
      break;
    case ARGS_VALUE_SKIP:
      stage = ARGS_VALUE_SKIP_EQ;
      break;
    default:
      break;
    }
    /* end of arg reached? */
    if( argv[na][++nc] == 0 )
    {
      /* next arg string */
      na++;
      /* start from begin */
      nc = 0;
      /* if no value supposed */
      if( stage != ARGS_VALUE && stage != ARGS_VALUE_SKIP )
      {
        /* start with none */
        stage = ARGS_NONE;
      }
      /* end of argv reached? */
      if( na == argc )
      {
        /* stop parsing */
        stage = ARGS_END;
      }
    }
  }
  /* return default value, if any */
  if( opt != NULL && need_value )
  {
    for( i = 0; i < nlist; i++ )
    {
      if( args_eq( opt, list[i] ) )
      {
        if( (s = strchr( list[i], '^' )) != NULL )
        {
          return s + 1;
        }
        break;
      }
    }
  }
  return NULL;
}

static char *args_help_buffer( char *list[], const int nlist )
{
  int i, j, k, w, c;
  char *s, *s1, *s2;
  char *buffer, *d, *d0;
  const char newline[] = "\n";

  /* help is divided in 2 columns: left for option's names, right for comments */
  /* now define width of left column (w) and size of all comments (c) */
  for( i = 0, k = 0, w = 0, c = 0; i < nlist; i++, k++ )
  {
    /* one short form option gives 2 literals */
    j = ((list[i][0] == '-') ? 2: 0);

    /* look for long form option */
    s1 = list[i] + j;
    if( s1[0] == '=' )
    {
      /* if short form was found */
      if( j != 0 )
      {
        /* append comma and space */
        j += 2;
      }
      s1++;
      if( (s2 = strchr( s1, '#' )) == NULL )
      {
        if( (s2 = strchr( s1, '%' )) == NULL )
        {
          s2 = s1 + strlen( s1 );
        }
      }
      /* add string length and 2 dashes */
      j += ((int)(s2 - s1) + 2);
    }
    /* value's name */
    if( (s1 = strchr( list[i], '#' )) != NULL )
    {
      s1++;
      if( (s2 = strchr( s1, '%' )) == NULL )
      {
        if( (s2 = strchr( s1, '^' )) == NULL )
        {
          s2 = s1 + strlen( s1 );
        }
      }
      /* name's length and equal sign */
      j += ((int)(s2 - s1) + 1);
    }
    /* keep maximum */
    if( j > w )
    {
      w = j;
    }
    /* if comments found */
    if( (s1 = strchr( list[i], '%')) != NULL )
    {
      s1++;
      if( (s2 = strchr( s1, '^' )) == NULL )
      {
        s2 = s1 + strlen( s1 );
      }
      for( s = strstr( s1, "\n" ); s != NULL; s = strstr( s, "\n" ), k++ ) s++;
      c += (int)(s2 - s1);
    }
    c += strlen( "\n" );
  }
  /* two spaces before and after */
  w += 4;
  /* buffer size */
  c += (k * w);
  if( (buffer = malloc( c + 256 )) == NULL )
    return NULL;
  memset( buffer, 0, c + 256 );

  /* now print help */
  for( i = 0, d = buffer; i < nlist; i++ )
  {
    /* origin position */
    d0 = d;
    /* one short form */
    s = NULL;
    j = 0;
    if( list[i][0] == '-' )
    {
      s = list[i];
      j = 2;
      *d++ = ' ';
      *d++ = ' ';
      *d++ = '-';
      *d++ = s[1];
    }
    /* look for long form option */
    s1 = list[i] + j;
    if( (s1[0] == '=' ) )
    {
      s1 = list[i] + j;
      *d++ = ((s != NULL) ? ',': ' ');
      *d++ = ' ';
      *d++ = '-';
      *d++ = '-';
      s1++;
      if( (s2 = strchr( s1, '#' )) == NULL )
      {
        if( (s2 = strchr( s1, '%' )) == NULL )
        {
          s2 = s1 + strlen( s1 );
        }
      }
      while( s1 != s2 )
      {
        *d++ = *s1++;
      }
    }
    /* value */
    if( (s1 = strchr( list[i], '#' )) != NULL )
    {
      s1++;
      if( (s2 = strchr( s1, '%' )) == NULL )
      {
        if( (s2 = strchr( s1, '^' )) == NULL )
        {
          s2 = s1 + strlen( s1 );
        }
      }
      *d++ = '=';
      while( s1 != s2 )
      {
        *d++ = *s1++;
      }
    }
    /* remain spaces to second column */
    d0 += w;
    while( d != d0 )
    {
      *d++ = ' ';
    }
    /* if comment found */
    if( (s1 = strchr( list[i], '%')) != NULL )
    {
      s1++;
      if( (s2 = strchr( s1, '^' )) == NULL )
      {
        s2 = s1 + strlen( s1 );
      }
      /* copy comment */
      while( s1 != s2 )
      {
        *d++ = *s1++;
        /* do not forget about new lines */
        if( (int)(d - buffer) >= (sizeof( newline ) - 1) )
        {
          if( memcmp( d - sizeof( newline ) + 1, newline, sizeof( newline ) - 1 ) == 0 )
          {
            /* align newlines */
            for( j = w; j != 0; j-- )
            {
              *d++ = ' ';
            }
          }
        }
      }
    }
    for( s = "\n"; *s != 0; )
    {
      *d++ = *s++;
    }
  }
  return buffer;
}

#define args_nlist (sizeof( args_list ) / sizeof( args_list[0] ))
#define args_arg() args_parse( argc, argv, args_list, args_nlist, NULL, 0, 0 )
#define args_argn( n ) args_parse( argc, argv, args_list, args_nlist, NULL, n, 0 )
#define args_opt( opt ) (args_parse( argc, argv, args_list, args_nlist, opt, 0, 0 ) != NULL)
#define args_val( opt ) args_parse( argc, argv, args_list, args_nlist, opt, 0, 1 )
#define args_valn( opt, n ) args_parse( argc, argv, args_list, args_nlist, opt, n, 1 )
#define args_check() (args_parse( argc, argv, args_list, args_nlist, NULL, -1, 0 ) == NULL && args_error_copt == 0 && args_error_sopt == NULL)
#define args_error() (args_error_copt != 0 ? (char *)&args_error_copt: args_error_sopt)
#define args_help() args_help_buffer( args_list, args_nlist )

#endif
