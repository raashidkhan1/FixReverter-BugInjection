import argparse
from collections import Counter
import json
from prettytable import PrettyTable


def count(path):
    with open(path, 'r') as f:
        j_data = json.load(f)
    patterns = [j['pattern'] for j in j_data]
    return Counter(patterns)


def parse():
    parser = argparse.ArgumentParser(description='Counts FixReverter patterns.')
    parser.add_argument('-y',
                        '--syntax',
                        help='Path to syntax matcher output apm.json.',
                        required=False)
    parser.add_argument('-m',
                        '--semantic',
                        help='Path to semantic matcher output apm.json.',
                        required=False)
    parser.add_argument('-f',
                        '--final',
                        help='Path to injector output inject.json.',
                        required=False)
    return parser.parse_args()


def main():
    parser = parse()
    if not parser.syntax and not parser.semantic and not parser.final:
        print('no input is given, exiting')
        exit(0)

    options = 0
    counts = []
    tab = PrettyTable()
    type_row = []

    if parser.syntax:
        counts.append(count(parser.syntax))
        options += 1
        type_row.append('syntax')
    if parser.semantic:
        counts.append(count(parser.semantic))
        options += 1
        type_row.append('semantic')
    if parser.final:
        counts.append(count(parser.final))
        options += 1
        type_row.append('final')

    tab.add_row([*(['ABORT'] * options), *(['EXEC'] * options), *(['ASSIGN'] * options)])
    tab.add_row(type_row * 3)
    data_row = []
    for p in ('COND_ABORT', 'COND_EXEC', 'COND_ASSIGN'):
        for c in counts:
            data_row.append(c[p])
    tab.add_row(data_row)
    print(tab)


if __name__ == '__main__':
    main()
