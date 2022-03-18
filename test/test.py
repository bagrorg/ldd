import os
import unittest
import re


class LDDTest(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self.ldd_path = '/home/bagrorg/Documents/GitHub/ldd/cmake-build-debug/ldd'


    def test1(self):
        self.process_fold('test_case1')

    def test2(self):
        self.process_fold('test_case2')

    def process_fold(self, fold):
        os.system(f"cd {fold} &&"
                  f"mkdir build && "
                  f"cd build && "
                  f"cmake .. && "
                  f"make")

        os.system(f'ldd {fold}/build/main > ldd_result.txt && LD_LIBRARY_PATH={fold}/build:$LD_LIBRARY_PATH  /{self.ldd_path} {fold}/build/main > my_result.txt')
        os.system(f'rm -rf {fold}/build')

        ans = open(f'ldd_result.txt', 'r')
        res = open(f'my_result.txt', 'r')


        ans_set = []
        for l in ans.readlines():
            if 'ld-linux-x86-64' in l:
                continue
            if 'linux-vdso' in l:
                continue
            ans_set.append(l[:-22])
        ans_set.sort()

        res_set = []
        for l in res.readlines():
            if 'ld-linux-x86-64' in l:
                continue
            res_set.append(l[:-1])
        res_set.sort()

        ans.close()
        res.close()

        os.system(f'rm -rf ldd_result.txt')
        os.system(f'rm -rf my_result.txt')

        self.assertEqual(ans_set, res_set)





