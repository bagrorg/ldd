import os
import unittest


class LDDTest(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self.ldd_path = '../build/ldd'

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

        stream = os.popen(
            f'LD_LIBRARY_PATH={fold}/build:$LD_LIBRARY_PATH ldd {fold}/build/main')

        ans = stream.read()
        stream = os.popen(f'LD_LIBRARY_PATH={fold}/build:$LD_LIBRARY_PATH {self.ldd_path} {fold}/build/main')
        res=stream.read()

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


        self.assertEqual(ans_set, res_set)
