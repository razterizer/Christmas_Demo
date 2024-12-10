#!/bin/bash

REPO_DIR = "Christmas_Demo"

brew install openal-soft

cd ..

./"${REPO_DIR}"/fetch-dependencies.py "${REPO_DIR}"/dependencies

cd "${REPO_DIR}/${REPO_DIR}"

./build.sh
