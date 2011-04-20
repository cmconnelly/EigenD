
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

#include "piw_powtable.h"

const unsigned piw::pow_table_size= 128;
//Look up table for the function 2 to the power of ((note-69)/12) used for frequency calculations. Values for note ranging from  0 to 127

#ifdef _WIN32
#pragma warning (disable : 4305)
#endif

float piw::pow_table[piw::pow_table_size] = 
{
0.0185813611719 , 0.0196862664046 , 0.0208568727214 , 0.0220970869121 , 0.0234110480762 , 0.024803141437 , 0.0262780129767 , 0.0278405849419 ,
0.0294960722713 , 0.03125 , 0.0331082216987 , 0.0350769390097 , 0.0371627223438 , 0.0393725328092 , 0.0417137454428 , 0.0441941738242 ,
0.0468220961524 , 0.049606282874 , 0.0525560259534 , 0.0556811698838 , 0.0589921445426 , 0.0625 , 0.0662164433975 , 0.0701538780193 ,
0.0743254446877 , 0.0787450656184 , 0.0834274908856 , 0.0883883476483 , 0.0936441923048 , 0.099212565748 , 0.105112051907 , 0.111362339768 ,
0.117984289085 , 0.125 , 0.132432886795 , 0.140307756039 , 0.148650889375 , 0.157490131237 , 0.166854981771 , 0.176776695297 ,
0.18728838461 , 0.198425131496 , 0.210224103813 , 0.222724679535 , 0.23596857817 , 0.25 , 0.26486577359 , 0.280615512077 ,
0.297301778751 , 0.314980262474 , 0.333709963543 , 0.353553390593 , 0.374576769219 , 0.396850262992 , 0.420448207627 , 0.44544935907 ,
0.471937156341 , 0.5 , 0.52973154718 , 0.561231024155 , 0.594603557501 , 0.629960524947 , 0.667419927085 , 0.707106781187 ,
0.749153538438 , 0.793700525984 , 0.840896415254 , 0.89089871814 , 0.943874312682 , 1.0 , 1.05946309436 , 1.12246204831 ,
1.189207115 , 1.25992104989 , 1.33483985417 , 1.41421356237 , 1.49830707688 , 1.58740105197 , 1.68179283051 , 1.78179743628 ,
1.88774862536 , 2.0 , 2.11892618872 , 2.24492409662 , 2.37841423001 , 2.51984209979 , 2.66967970834 , 2.82842712475 ,
2.99661415375 , 3.17480210394 , 3.36358566101 , 3.56359487256 , 3.77549725073 , 4.0 , 4.23785237744 , 4.48984819324 ,
4.75682846001 , 5.03968419958 , 5.33935941668 , 5.65685424949 , 5.99322830751 , 6.34960420787 , 6.72717132203 , 7.12718974512 ,
7.55099450145 , 8.0 , 8.47570475487 , 8.97969638647 , 9.51365692002 , 10.0793683992 , 10.6787188334 , 11.313708499 ,
11.986456615 , 12.6992084157 , 13.4543426441 , 14.2543794902 , 15.1019890029 , 16.0 , 16.9514095097 , 17.9593927729 ,
19.02731384 , 20.1587367983 , 21.3574376667 , 22.627416998 , 23.97291323 , 25.3984168315 , 26.9086852881 , 28.5087589805 ,
}; 

