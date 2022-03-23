import os
import unittest


class LDDTest(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self.ldd_path = '../build/ldd'

    def test1(self):
        self.process_fold('test_case1', True)

    def test2(self):
        self.process_fold('test_case2', True)

    def test3(self):
        self.process_bin('/usr/bin/gdb')

    def test4(self):
        self.process_bin('/usr/bin/clang')

    def test5(self):
        self.process_bin('/usr/bin/g++')


    def process_fold(self, fold, to_build):
        if to_build:
            os.system(f"cd {fold} &&"
                  f"mkdir build && "
                  f"cd build && "
                  f"cmake .. && "
                  f"make")

        stream = os.popen(
            f'LD_LIBRARY_PATH={fold}/build:$LD_LIBRARY_PATH ldd {fold}/build/main')

        ans = stream.read()
        stream = os.popen(f'LD_LIBRARY_PATH={fold}/build:$LD_LIBRARY_PATH {self.ldd_path} {fold}/build/main')
        res=stream.read()

        if to_build:
            os.system(f'rm -rf {fold}/build')


        ans_set = []
        for l in ans.split('\n'):
            if 'ld-linux-x86-64' in l:
                continue
            if 'linux-vdso' in l:
                continue
            ans_set.append(l[:-21])
        ans_set.sort()

        res_set = []
        for l in res.split('\n'):
            if 'ld-linux-x86-64' in l:
                continue
            res_set.append(l)
        res_set.sort()

        for ans, res in zip(ans_set, res_set):
            path_ans, path_res = ans.split('=>'), res.split('=>')
            so_ans, so_res = path_ans[-1].split('/')[-1], path_res[-1].split('/')[-1]
            self.assertEqual(so_ans, so_res)

    def process_bin(self, bin):
        stream = os.popen(f'ldd {bin}')

        ans = stream.read()
        stream = os.popen(f'{self.ldd_path} {bin}')
        res = stream.read()

        ans_set = []
        for l in ans.split('\n'):
            if 'ld-linux-x86-64' in l:
                continue
            if 'linux-vdso' in l:
                continue
            ans_set.append(l[:-21])
        ans_set.sort()

        res_set = []
        for l in res.split('\n'):
            if 'ld-linux-x86-64' in l:
                continue
            res_set.append(l)
        res_set.sort()

        for ans, res in zip(ans_set, res_set):
            path_ans, path_res = ans.split('=>'), res.split('=>')
            so_ans, so_res = path_ans[-1].split('/')[-1], path_res[-1].split('/')[-1]
            self.assertEqual(so_ans, so_res)
