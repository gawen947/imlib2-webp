#!/bin/sh
# script to determine the version of imlib2

if ! which imlib2-config > /dev/null
then
  >&2 echo "error: cannot find imlib2-config (required to find Imlib2 version)."
  exit 1
fi

version=$(imlib2-config --version)
major=$(echo "$version" | cut -d'.' -f1)
minor=$(echo "$version" | cut -d'.' -f2)
patch=$(echo "$version" | cut -d'.' -f3)

echo "-DIMLIB2_VERSION_MAJOR=$major -DIMLIB2_VERSION_MINOR=$minor -DIMLIB2_VERSION_PATCH=$patch"
