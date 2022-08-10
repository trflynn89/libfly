#!/usr/bin/env python3

import argparse
import hashlib
import json
import os
import pathlib
import requests
import shutil

from typing import Callable
from urllib.parse import urlparse


def extract_archive(path: pathlib.Path):
    shutil.unpack_archive(path, path.parent)


def sha256_hash(path: pathlib.Path):
    hasher = hashlib.sha256()

    with path.open('rb') as file:
        while chunk := file.read(1 << 20):
            hasher.update(chunk)

    return hasher.hexdigest()


def git_hash(path: pathlib.Path):
    hasher = hashlib.sha1()

    size = path.stat().st_size
    hasher.update(f'blob {size}\0'.encode('utf-8'))

    with path.open('rb') as file:
        while chunk := file.read(1 << 20):
            hasher.update(chunk)

    return hasher.hexdigest()


def download_source(url: str, destination: pathlib.Path, sha: str, hasher: Callable[[pathlib.Path], str], session: requests.Session = None):
    if destination.is_file():
        if hasher(destination) == sha:
            return False

        destination.unlink()

    destination.parent.mkdir(parents=True, exist_ok=True)

    connection = session if session else requests
    response = connection.get(url, stream=True)

    with destination.open('wb') as destination_file:
        for chunk in response.iter_content(1 << 20):
            destination_file.write(chunk)

    assert hasher(destination) == sha
    return True


def download_github_sources(sources: pathlib.Path):
    with open(sources, 'rb') as sources_file:
        contents = json.load(sources_file)

    session = requests.Session()

    for file in contents:
        destination = sources.parent.joinpath(file['name'])
        url = file['download_url']
        sha1 = file['sha']

        download_source(url, destination, sha1, git_hash, session=session)


DATA_SOURCES = [
    (
        'bench',
        'http://mattmahoney.net/dc/enwik8.zip',
        '547994d9980ebed1288380d652999f38a14fe291a6247c157c3d33d4932534bc',
        pathlib.Path('coders'),
        extract_archive,
    ),
    (
        'bench',
        'https://raw.githubusercontent.com/miloyip/nativejson-benchmark/54776ce82317daffaec3a5526919587981a654ab/data/canada.json',
        'f83b3b354030d5dd58740c68ac4fecef64cb730a0d12a90362a7f23077f50d78',
        pathlib.Path('json'),
        None,
    ),
    (
        'test',
        'https://raw.githubusercontent.com/nlohmann/json_test_data/a1375cea09d27cc1c4cadb8d00470375b421ac37/json_nlohmann_tests/all_unicode.json',
        '4722e1348ee7931d4099ec5d6bddada335bae416ffd37a159a669b48e3c8d251',
        pathlib.Path('json'),
        None,
    ),
    (
        'test',
        'https://raw.githubusercontent.com/minimaxir/big-list-of-naughty-strings/db33ec7b1d5d9616a88c76394b7d0897bd0b97eb/blns.json',
        'b5edb4dffb234fa8b37c6353ec2cbd414ce721a03968d26343a7c276ab360f63',
        pathlib.Path('json'),
        None,
    ),
    (
        'test',
        'https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/json-test-suite/sample.zip',
        '4b48f4fd2d66db5ba80f86f3d08b0de4921d1e01ab9ee65a66f90e46f19126f3',
        pathlib.Path('json').joinpath('google_json_test_suite'),
        extract_archive,
    ),
    (
        'test',
        'https://www.json.org/JSON_checker/test.zip',
        '5abaf51d15cd19beb36db16a659bc4fe298c5e5f870f5ae0d6831ea2eb7c02c0',
        pathlib.Path('json').joinpath('json_checker'),
        extract_archive,
    ),
    (
        'test',
        'https://api.github.com/repos/nst/JSONTestSuite/contents/test_parsing?ref=d64aefb55228d9584d3e5b2433f720ea8fd00c82',
        'f70f41929c27d7786f0c0f40a73434c84df1159bccd6b0db8959bbfbeb6a443e',
        pathlib.Path('json').joinpath('nst_json_test_suite'),
        download_github_sources,
    ),
]


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        'generated_sources_directory', nargs=1, type=pathlib.Path,
        help='Path to the directory containing generated source files.')
    parser.add_argument(
        'data_directory', nargs=1, type=pathlib.Path,
        help='Path to the directory containing data files.')
    parser.add_argument(
        'data_type', nargs=1, choices=('bench', 'test'),
        help='Type of data to download.')

    args = parser.parse_args()

    data_directory = args.data_directory[0]
    data_type = args.data_type[0]

    for (type, url, sha256, subdir, post_download_filter) in DATA_SOURCES:
        if type != data_type:
            continue

        file = pathlib.Path(urlparse(url).path).name
        destination = data_directory.joinpath(subdir, file)

        file_was_downloaded = download_source(
            url, destination, sha256, sha256_hash)

        if file_was_downloaded and (post_download_filter is not None):
            post_download_filter(destination)


if __name__ == '__main__':
    main()
