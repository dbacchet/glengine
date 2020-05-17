#include <cstring>

#include "catch2/catch.hpp"

#include "fcontext/fcontext.h"

namespace {

fcontext_t ctx1;
fcontext_t ctx2;

static void foo(fcontext_transfer_t t)
{
    char *mystr = (char*)t.data;
    strcat(mystr,"FOO_");
    jump_fcontext(ctx2, t.data);
    strcat(mystr,"FOO2_");
    jump_fcontext(t.ctx, t.data);
}

static void doo(fcontext_transfer_t t)
{
    char *mystr = (char*)t.data;
    strcat(mystr,"DOO_");
    jump_fcontext(t.ctx, t.data);
}

} // empty namespace 

TEST_CASE("fcontext coroutines", "[fcontext]") {
    fcontext_stack_t s1 = create_fcontext_stack(16 * 1024);
    fcontext_stack_t s2 = create_fcontext_stack(4 * 1024);

    ctx1 = make_fcontext(s1.sptr, s1.ssize, foo);
    ctx2 = make_fcontext(s2.sptr, s2.ssize, doo);

    char mystring[32] = {0};
    jump_fcontext(ctx1, (void*)mystring);
    strcat(mystring,"END");

    REQUIRE(strcmp(mystring, "FOO_DOO_FOO2_END")==0);

    destroy_fcontext_stack(&s1);
    destroy_fcontext_stack(&s2);
}
