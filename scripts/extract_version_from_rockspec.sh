#!/bin/sh

# Simple script for version extraction from rockspec file
# Copyright (c) Yassn Achengli

rockspec_file=$(ls $(dirname $0)/.. | grep -i leregex | xargs)
version=$(cat $(dirname $0)/../$rockspec_file | grep -i version | cut -d= -f2 | grep -oE "[0-9]\.[0-9]-[0-9]")

version_to_number(){
    local major=$(echo $1 | cut -d'.' -f1)
    local minor=$(echo $1 | cut -d'.' -f2 | cut -d'-' -f1)
    local release=$(echo $1 | cut -d'-' -f2)
    echo $(( $release + $minor * 100 + $major * 10000 ))
}

version_make_flags(){
    local major=$(echo $1 | cut -d'.' -f1)
    local minor=$(echo $1 | cut -d'.' -f2 | cut -d'-' -f1)
    local release=$(echo $1 | cut -d'-' -f2)
    echo "-MAJOR=${major} -MINOR=${minor} -RELEASE=${release}"
}

version_export(){
    local path=$(dirname $0)/../src/version.h
    local version_number=$(version_to_number $version)
    local version_flags=$(version_make_flags $version)    
    local define_version_number=
    local define_version_subs=$(echo $version_flags | tr " " "\n")

    echo "#ifndef LEREGEX_VERSION_H" > $path
    echo "#define LEREGEX_VERSION_H" >> $path
    echo "" >> $path
    echo "#define LEREGEX_VERSION ${version_number}" >> $path
    echo "" >> $path
    for flag in $version_flags; do
        echo $flag | sed -E s'/-([A-Z]+)=/\#define LEREGEX_\1 /g' >> $path
    done
    echo "#endif" >> $path
}

if [ -n "$1" ]; then
    case "$1" in
        --number|number|-n)
            version_to_number $version ;;
        --make)
            version_make_flags $version;;
        --export)
            version_export $version;;
        *)
            echo $version;;
    esac
else
    echo $version
fi
