#!/bin/sh
# script to determine the version of imlib2

version=$(pkg-config imlib2 --version)
major=$(echo "$version" | cut -d'.' -f1)
minor=$(echo "$version" | cut -d'.' -f2)
patch=$(echo "$version" | cut -d'.' -f3)

echo "-DIMLIB2_VERSION_MAJOR=$major -DIMLIB2_VERSION_MINOR=$minor -DIMLIB2_VERSION_PATCH=$patch"
