#include <regex.h>
#include <stdio.h>

int main()
{
  regex_t rx;
  unsigned idx;
  regmatch_t m[3];

  for (idx = 0; idx < 3; idx++){
    printf("match: %d,%d\n", m[idx].rm_so, m[idx].rm_eo);
  }

  if (regcomp(&rx, "[0-9]+_[0-9]+", REG_EXTENDED) != 0) {
    printf("invalid regular expression\n");
  }

  if (regexec(&rx, 
        "hello there 2839_289 having notes of 782_27878 and 2348_990",
        3, m, REG_EXTENDED) == 0) {
    for (idx = 0; idx < 3; idx++){
      printf("match: %d,%d\n", m[idx].rm_so, m[idx].rm_eo);
    }
  } else {
    printf("regex failed\n");
  }
  return 0;
}
