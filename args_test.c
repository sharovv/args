#include "args.h"

/*
 * Test with: -x --flag --warning-level=10 arg1 --option opt1 arg2 -o opt2 --option=opt3
 */
static int test()
{
  char *argv[] = { "cmdarg", "-x", "--flag", "--warning-level=10", "arg1", "--option", "opt1", "arg2", "-o", "opt2", "--option=opt3" };
  int argc = sizeof( argv ) / sizeof( argv[0] );
  static char *args_list[] = {
    "-f=flag%Flag",
    "-x=aux%Auxiliary flag",
    "-w=warning-level#LEVEL%Warning level^3",
    "-o=option#VALUE%Additional option(s)"
  };
  int f, x;
  char *w, *o1, *o2, *o3, *o4, *a, *a1, *a2, *a3;

  if( !args_check() )
  {
    fprintf( stderr, "Unknown option - \"%s\"\n", args_error() );
    return 0;
  }

  f = args_opt( "-f" );
  x = args_opt( "-x" );

  w = args_val( "-w" );

  o1 = args_valn( "-o", 0 );
  o2 = args_valn( "-o", 1 );
  o3 = args_valn( "-o", 2 );
  o4 = args_valn( "-o", 3 );

  a = args_arg();
  a1 = args_argn( 0 );
  a2 = args_argn( 1 );
  a3 = args_argn( 2 );

  return (f == 1 && x == 1 &&
      strcmp( w, "10" ) == 0 &&
      strcmp( o1, "opt1" ) == 0 && strcmp( o2, "opt2" ) == 0 && strcmp( o3, "opt3" ) == 0 && o4 == NULL &&
      strcmp( a, "arg1" ) == 0 &&
      strcmp( a1, "arg1" ) == 0 && strcmp( a2, "arg2" ) == 0 && a3 == NULL);
}


int main( int argc, char *argv[] )
{
  static char *args_list[] = {
    "-f=flag%Flag",
    "-x=aux%Auxiliary flag",
    "-w=warning-level#LEVEL%Warning level^3",
    "-o=option#VALUE%Additional option(s),\nand multiline comment.",
    "-?=help%Print help"
  };
  int i;
  char *s;

  if( args_opt( "-?" ) )
  {
    printf( "Usage: test_args [OPTIONS..] [ARGS...]\n"
            "Test args.h library\n\n"
            "Options:\n"
            "%s\n", args_help() );
    return 0;
  }

  if( !args_check() )
  {
    printf( "Unknown option - \"%s\"\n", args_error() );
    return 1;
  }

  if( !test() )
  {
    fprintf( stderr, "test failed\n" );
  }

  s = args_arg();
  printf( "First argument: %s\n", s != NULL ? s: "<missing>" );

  for( i = 0; ; i++ )
  {
    if( (s = args_argn( i )) == NULL )
      break;
    printf( "%s\"%s\"", i == 0 ? "All arguments: ": ", ", s );
  }
  if( i ) printf( "\n" );

  if( args_opt( "-f" ) )
  {
    printf( "Flag enabled\n" );
  }

  if( args_opt( "-x" ) )
  {
    printf( "Auxiliary flag enabled\n" );
  }

  s = args_val( "-w" );
  printf( "Warning level: %s\n", s );

  for( i = 0; ; i++ )
  {
    if( (s = args_valn( "-o", i )) == NULL )
      break;
    printf( "%s\"%s\"", i == 0 ? "Additional option(s): ": ", ", s );
  }
  if( i ) printf( "\n" );
  return 0;
}
