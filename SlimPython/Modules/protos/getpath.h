static void reduce ( char *dir );
static int isfile ( char *filename );
static int ismodule ( char *filename );
static int isxfile ( char *filename );
static int isdir ( char *filename );
static void joinpath ( char *buffer , char *stuff );
static int search_for_prefix ( char *argv0_path , char *home );
static int search_for_exec_prefix ( char *argv0_path , char *home );
static void calculate_path ( void );
