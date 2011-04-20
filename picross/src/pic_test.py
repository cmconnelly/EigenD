
#
# Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com
#
# This file is part of EigenD.
#
# EigenD is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# EigenD is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
#

import unittest
import pic_test_native

class PicrossTest(unittest.TestCase):
    def test_atomic_basic(self):
        pic_test_native.test_atomic_basic()

    def test_atomic_threaded(self):
        pic_test_native.test_atomic_threaded()

    def test_gate(self):
        pic_test_native.test_gate()

    def test_dotprod(self):
        pic_test_native.test_dotprod()

if __name__ == '__main__':
    unittest.main()
