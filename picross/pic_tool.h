
/*
 Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com

 This file is part of EigenD.

 EigenD is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 EigenD is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __PICROSS_TOOL__
#define __PICROSS_TOOL__

#include <pic_exports.h>
#include <string>

namespace pic
{
    class PIC_DECLSPEC_CLASS tool_t
    {
        public:
            class impl_t;
        public:
            tool_t(const char *dir,const char *name);
            tool_t(const std::string &dir,const char *name);
            ~tool_t();
            void start();
            bool isrunning();
            void quit();
        private:
            impl_t *impl_;
    };
};

#endif
