# coding: utf8

COIN = 10 ** 8


def calc_block_reward(mode, height):
    half_blocks_count = 210000
    if mode == 'regtest':
        half_blocks_count = 150
    halvings = int(height / half_blocks_count)
    if halvings >= 64:
        return 0
    n_subsidy = 50 * COIN
    n_subsidy = n_subsidy >> halvings
    return n_subsidy
