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

#ifndef __PIW_SCLONE__
#define __PIW_SCLONE__
#include "piw_exports.h"
#include "piw_bundle.h"
#include "piw_clock.h"
#include "piw_data.h"

namespace piw
{
    class PIW_DECLSPEC_CLASS sclone_t
    {
        public:
            class impl_t;
        public:
            sclone_t();
            ~sclone_t();
            void set_output(unsigned name, const cookie_t &);
            void set_filtered_output(unsigned name, const cookie_t &, const piw::d2b_t &);
            void clear_output(unsigned name);
            cookie_t cookie();
            int gc_traverse(void *, void *) const;
            int gc_clear();
        private:
            impl_t *_root;
    };

    PIW_DECLSPEC_FUNC(d2b_t) first_filter(unsigned);
};

#endif
