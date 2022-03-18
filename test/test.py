import os
import unittest


class LDDTest(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self.ldd_path = '${{github.workspace}}/build'

    def test1(self):
        self.process_fold('test_case1', 1)

    def test2(self):
        self.process_fold('test_case2', 2)

    def process_fold(self, fold, id):
        os.system(f"cd {fold} &&"
                  f"mkdir build && "
                  f"cd build && "
                  f"cmake .. && "
                  f"make")

        os.system(
            f'LD_LIBRARY_PATH={fold}/build:$LD_LIBRARY_PATH ldd {fold}/build/main > ldd_result{id}.txt && '
            f'LD_LIBRARY_PATH={fold}/build:$LD_LIBRARY_PATH  /{self.ldd_path} {fold}/build/main > my_result{id}.txt')
        os.system(f'rm -rf {fold}/build')

        ans = open(f'ldd_result{id}.txt', 'r')
        res = open(f'my_result{id}.txt', 'r')

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

        os.system(f'rm -rf ldd_result{id}.txt')
        os.system(f'rm -rf my_result{id}.txt')

        self.assertEqual(ans_set, res_set)
