package = "leregex"
version = "1.0.0"

source = {
    url = "git+https://github.com/achengli/leregex"
}

description = {
    summary = "Lua regex.h C regular expressions extension",
    detailed = "Lua regex.h C regular expressions extension",
    homepage = "https://github.com/achengli/leregex",
    license = "BSD",
    maintainer = "yassin_achengli@hotmail.com"
}

dependencies = {
    "lua >= 5.1",
}

external_dependencies = {
    LEREGEX = {
        header = "leregex/src/leregex.h",
    }
}

build = {
    type = "builtin",
    modules = {
        leregex = {
            sources = {"src/leregex.c"},
            libraries = {"src/leregex"},
            incdirs = {"$(LEREGEX_INCDIR)"},
            libdirs = {"$(LEREGEX_LIBDIR)"},
        }
    },
    copy_directories = {
        "test",
        "src",
    }
}
