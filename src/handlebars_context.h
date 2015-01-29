
/**
 * @file
 * @brief Context to hold parser and lexer state
 */

#ifndef HANDLEBARS_CONTEXT_H
#define HANDLEBARS_CONTEXT_H

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Pre-declarations
 */
struct handlebars_ast_node;
struct YYLTYPE;

/**
 * Contains the scanning state information
 */
struct handlebars_context
{
  char * tmpl;
  int tmplReadOffset;
  void * scanner;
  struct handlebars_ast_node * program;
  int eof;
  int errnum;
  char * error;
  struct YYLTYPE * errloc;
};

struct handlebars_context * handlebars_context_ctor(void);
void handlebars_context_dtor(struct handlebars_context * context);
char * handlebars_context_get_errmsg(struct handlebars_context * context);
char * handlebars_context_get_errmsg_js(struct handlebars_context * context);

#ifdef	__cplusplus
}
#endif

#endif