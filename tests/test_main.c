

#include <check.h>
#include <talloc.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "handlebars.h"
#include "handlebars_context.h"
#include "handlebars_token.h"
#include "handlebars_token_list.h"
#include "handlebars_token_printer.h"
#include "handlebars.tab.h"
#include "handlebars.lex.h"
#include "utils.h"

START_TEST(test_version)
{
	ck_assert_int_eq(handlebars_version(), HANDLEBARS_VERSION_PATCH
	        + (HANDLEBARS_VERSION_MINOR * 100)
	        + (HANDLEBARS_VERSION_MAJOR * 10000));
}
END_TEST

START_TEST(test_version_string)
{
	int maj = 0, min = 0, rev = 0;
	const char * version_string = handlebars_version_string();

	sscanf(version_string, "%d.%d.%d", &maj, &min, &rev);
	ck_assert_int_eq(maj, HANDLEBARS_VERSION_PATCH);
	ck_assert_int_eq(min, HANDLEBARS_VERSION_MINOR);
	ck_assert_int_eq(rev, HANDLEBARS_VERSION_MAJOR);
}
END_TEST

START_TEST(test_lex)
{
    struct handlebars_context * ctx = handlebars_context_ctor();
    struct handlebars_token_list * list;
    
    ctx->tmpl = "{{foo}}";
    
    list = handlebars_lex(ctx);
    
    ck_assert_ptr_ne(NULL, list->first);
    ck_assert_int_eq(OPEN, list->first->data->token);
    ck_assert_str_eq("{{", list->first->data->text);
    
    ck_assert_ptr_ne(NULL, list->first->next);
    ck_assert_int_eq(ID, list->first->next->data->token);
    ck_assert_str_eq("foo", list->first->next->data->text);
    
    ck_assert_ptr_ne(NULL, list->first->next->next);
    ck_assert_int_eq(CLOSE, list->first->next->next->data->token);
    ck_assert_str_eq("}}", list->first->next->next->data->text);
}
END_TEST

Suite * parser_suite(void)
{
  Suite * s = suite_create("Handlebars");

  REGISTER_TEST(s, test_version, "Version");
  REGISTER_TEST(s, test_version_string, "Version String");
  REGISTER_TEST(s, test_lex, "Lex Convenience Function");

  return s;
}

int main(void)
{
  int number_failed;
  Suite * s = parser_suite();
  SRunner * sr = srunner_create(s);
#if defined(_WIN64) || defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN32__)
  srunner_set_fork_status(sr, CK_NOFORK);
#endif
  //srunner_set_log(sr, "test_token.log");
  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
