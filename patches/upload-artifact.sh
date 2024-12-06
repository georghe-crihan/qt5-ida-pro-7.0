#!/bin/sh

upload()
{
local FILE="${1}"
local RELEASE="${2}"
local GITHUB_TOKEN="${3}"

curl \
    -H "Authorization: token ${GITHUB_TOKEN}" \
    -H "Content-Type: $(file -b --mime-type ${FILE})" \
    --data-binary @${FILE} \
    "https://uploads.github.com/repos/qeorghe-crihan/qt5-ida-pro-7.0/releases/${RELEASE}/assets?name=$(basename ${FILE})"
}

upload "${1}" "${2}" "${3}"
