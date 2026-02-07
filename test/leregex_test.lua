local function script_path()
   local str = debug.getinfo(2, "S").source:sub(2)
   return str:match("(.*/)") or "./"
end

local test_path = script_path()

package.path    = test_path .. "../src/?.lua;" .. package.path
package.cpath   = test_path .. "../src/?.so;" .. package.cpath

local leregex = require'leregex'

local p = "jkenjke hello ikebnrjk jkbne hello"
local r = leregex.r("hello")
local m = leregex.match_all(p, r)

print_table(leregex)

for _, v in pairs(m) do
    assert(string.sub(p, v.from, v.to) == v.match)
end
print('test: OK')
