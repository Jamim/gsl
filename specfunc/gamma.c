/* Author:  G. Jungman
 * RCS:     $Id$
 */
#include <stdio.h>
#include <math.h>
#include <gsl_errno.h>
#include <gsl_math.h>
#include "gsl_sf_log.h"
#include "gsl_sf_trig.h"
#include "gsl_sf_gamma.h"

#define LogRootTwoPi_  0.9189385332046727418
#define LogPi_         1.1447298858494001741
#define Max(a,b) ((a) > (b) ? (a) : (b))


/*-*-*-*-*-*-*-*-*-*-*-* Private Section *-*-*-*-*-*-*-*-*-*-*-*/

#define FACT_TABLE_MAX  170
#define FACT_TABLE_SIZE (FACT_TABLE_MAX+1)
static struct {int n; double f; long i; } fact_table[FACT_TABLE_SIZE] = {
    { 0,  1.0,     1L     },
    { 1,  1.0,     1L     },
    { 2,  2.0,     2L     },
    { 3,  6.0,     6L     },
    { 4,  24.0,    24L    },
    { 5,  120.0,   120L   },
    { 6,  720.0,   720L   },
    { 7,  5040.0,  5040L  },
    { 8,  40320.0, 40320L },

    { 9,  362880.0,     362880L    },
    { 10, 3628800.0,    3628800L   },
    { 11, 39916800.0,   39916800L  },
    { 12, 479001600.0,  479001600L },

    { 13, 6227020800.0,                               0 },
    { 14, 87178291200.0,                              0 },
    { 15, 1307674368000.0,                            0 },
    { 16, 20922789888000.0,                           0 },
    { 17, 355687428096000.0,                          0 },
    { 18, 6402373705728000.0,                         0 },
    { 19, 121645100408832000.0,                       0 },
    { 20, 2432902008176640000.0,                      0 },
    { 21, 51090942171709440000.0,                     0 },
    { 22, 1124000727777607680000.0,                   0 },
    { 23, 25852016738884976640000.0,                  0 },
    { 24, 620448401733239439360000.0,                 0 },
    { 25, 15511210043330985984000000.0,               0 },
    { 26, 403291461126605635584000000.0,              0 },
    { 27, 10888869450418352160768000000.0,            0 },
    { 28, 304888344611713860501504000000.0,           0 },
    { 29, 8841761993739701954543616000000.0,          0 },
    { 30, 265252859812191058636308480000000.0,        0 },
    { 31, 8222838654177922817725562880000000.0,       0 },
    { 32, 263130836933693530167218012160000000.0,     0 },
    { 33, 8683317618811886495518194401280000000.0,    0 },
    { 34, 2.95232799039604140847618609644e38,  0 },
    { 35, 1.03331479663861449296666513375e40,  0 },
    { 36, 3.71993326789901217467999448151e41,  0 },
    { 37, 1.37637530912263450463159795816e43,  0 },
    { 38, 5.23022617466601111760007224100e44,  0 },
    { 39, 2.03978820811974433586402817399e46,  0 },
    { 40, 8.15915283247897734345611269600e47,  0 },
    { 41, 3.34525266131638071081700620534e49,  0 },
    { 42, 1.40500611775287989854314260624e51,  0 },
    { 43, 6.04152630633738356373551320685e52,  0 },
    { 44, 2.65827157478844876804362581101e54,  0 },
    { 45, 1.19622220865480194561963161496e56,  0 },
    { 46, 5.50262215981208894985030542880e57,  0 },
    { 47, 2.58623241511168180642964355154e59,  0 },
    { 48, 1.24139155925360726708622890474e61,  0 },
    { 49, 6.08281864034267560872252163321e62,  0 },
    { 50, 3.04140932017133780436126081661e64,  0 },
    { 51, 1.55111875328738228022424301647e66,  0 },
    { 52, 8.06581751709438785716606368564e67,  0 },
    { 53, 4.27488328406002556429801375339e69,  0 },
    { 54, 2.30843697339241380472092742683e71,  0 },
    { 55, 1.26964033536582759259651008476e73,  0 },
    { 56, 7.10998587804863451854045647464e74,  0 },
    { 57, 4.05269195048772167556806019054e76,  0 },
    { 58, 2.35056133128287857182947491052e78,  0 },
    { 59, 1.38683118545689835737939019720e80,  0 },
    { 60, 8.32098711274139014427634118320e81,  0 },
    { 61, 5.07580213877224798800856812177e83,  0 },
    { 62, 3.14699732603879375256531223550e85,  0 },
    { 63, 1.982608315404440064116146708360e87,  0 },
    { 64, 1.268869321858841641034333893350e89,  0 },
    { 65, 8.247650592082470666723170306800e90,  0 },
    { 66, 5.443449390774430640037292402480e92,  0 },
    { 67, 3.647111091818868528824985909660e94,  0 },
    { 68, 2.480035542436830599600990418570e96,  0 },
    { 69, 1.711224524281413113724683388810e98,  0 },
    { 70, 1.197857166996989179607278372170e100,  0 },
    { 71, 8.504785885678623175211676442400e101,  0 },
    { 72, 6.123445837688608686152407038530e103,  0 },
    { 73, 4.470115461512684340891257138130e105,  0 },
    { 74, 3.307885441519386412259530282210e107,  0 },
    { 75, 2.480914081139539809194647711660e109,  0 },
    { 76, 1.885494701666050254987932260860e111,  0 },
    { 77, 1.451830920282858696340707840860e113,  0 },
    { 78, 1.132428117820629783145752115870e115,  0 },
    { 79, 8.946182130782975286851441715400e116,  0 },
    { 80, 7.156945704626380229481153372320e118,  0 },
    { 81, 5.797126020747367985879734231580e120,  0 },
    { 82, 4.753643337012841748421382069890e122,  0 },
    { 83, 3.945523969720658651189747118010e124,  0 },
    { 84, 3.314240134565353266999387579130e126,  0 },
    { 85, 2.817104114380550276949479442260e128,  0 },
    { 86, 2.422709538367273238176552320340e130,  0 },
    { 87, 2.107757298379527717213600518700e132,  0 },
    { 88, 1.854826422573984391147968456460e134,  0 },
    { 89, 1.650795516090846108121691926250e136,  0 },
    { 90, 1.485715964481761497309522733620e138,  0 },
    { 91, 1.352001527678402962551665687590e140,  0 },
    { 92, 1.243841405464130725547532432590e142,  0 },
    { 93, 1.156772507081641574759205162310e144,  0 },
    { 94, 1.087366156656743080273652852570e146,  0 },
    { 95, 1.032997848823905926259970209940e148,  0 },
    { 96, 9.916779348709496892095714015400e149,  0 },
    { 97, 9.619275968248211985332842594960e151,  0 },
    { 98, 9.426890448883247745626185743100e153,  0 },
    { 99, 9.332621544394415268169923885600e155,  0 },
    { 100, 9.33262154439441526816992388563e157,  0 },
    { 101, 9.42594775983835942085162312450e159,  0 },
    { 102, 9.61446671503512660926865558700e161,  0 },
    { 103, 9.90290071648618040754671525458e163,  0 },
    { 104, 1.02990167451456276238485838648e166,  0 },
    { 105, 1.08139675824029090050410130580e168,  0 },
    { 106, 1.146280563734708354534347384148e170,  0 },
    { 107, 1.226520203196137939351751701040e172,  0 },
    { 108, 1.324641819451828974499891837120e174,  0 },
    { 109, 1.443859583202493582204882102460e176,  0 },
    { 110, 1.588245541522742940425370312710e178,  0 },
    { 111, 1.762952551090244663872161047110e180,  0 },
    { 112, 1.974506857221074023536820372760e182,  0 },
    { 113, 2.231192748659813646596607021220e184,  0 },
    { 114, 2.543559733472187557120132004190e186,  0 },
    { 115, 2.925093693493015690688151804820e188,  0 },
    { 116, 3.393108684451898201198256093590e190,  0 },
    { 117, 3.96993716080872089540195962950e192,  0 },
    { 118, 4.68452584975429065657431236281e194,  0 },
    { 119, 5.57458576120760588132343171174e196,  0 },
    { 120, 6.68950291344912705758811805409e198,  0 },
    { 121, 8.09429852527344373968162284545e200,  0 },
    { 122, 9.87504420083360136241157987140e202,  0 },
    { 123, 1.21463043670253296757662432419e205,  0 },
    { 124, 1.50614174151114087979501416199e207,  0 },
    { 125, 1.88267717688892609974376770249e209,  0 },
    { 126, 2.37217324288004688567714730514e211,  0 },
    { 127, 3.01266001845765954480997707753e213,  0 },
    { 128, 3.85620482362580421735677065923e215,  0 },
    { 129, 4.97450422247728744039023415041e217,  0 },
    { 130, 6.46685548922047367250730439554e219,  0 },
    { 131, 8.47158069087882051098456875820e221,  0 },
    { 132, 1.11824865119600430744996307608e224,  0 },
    { 133, 1.48727070609068572890845089118e226,  0 },
    { 134, 1.99294274616151887673732419418e228,  0 },
    { 135, 2.69047270731805048359538766215e230,  0 },
    { 136, 3.65904288195254865768972722052e232,  0 },
    { 137, 5.01288874827499166103492629211e234,  0 },
    { 138, 6.91778647261948849222819828311e236,  0 },
    { 139, 9.61572319694108900419719561353e238,  0 },
    { 140, 1.34620124757175246058760738589e241,  0 },
    { 141, 1.89814375907617096942852641411e243,  0 },
    { 142, 2.69536413788816277658850750804e245,  0 },
    { 143, 3.85437071718007277052156573649e247,  0 },
    { 144, 5.55029383273930478955105466055e249,  0 },
    { 145, 8.04792605747199194484902925780e251,  0 },
    { 146, 1.17499720439091082394795827164e254,  0 },
    { 147, 1.72724589045463891120349865931e256,  0 },
    { 148, 2.55632391787286558858117801578e258,  0 },
    { 149, 3.80892263763056972698595524351e260,  0 },
    { 150, 5.71338395644585459047893286526e262,  0 },
    { 151, 8.62720977423324043162318862650e264,  0 },
    { 152, 1.31133588568345254560672467123e267,  0 },
    { 153, 2.00634390509568239477828874699e269,  0 },
    { 154, 3.08976961384735088795856467036e271,  0 },
    { 155, 4.78914290146339387633577523906e273,  0 },
    { 156, 7.47106292628289444708380937294e275,  0 },
    { 157, 1.17295687942641442819215807155e278,  0 },
    { 158, 1.85327186949373479654360975305e280,  0 },
    { 159, 2.94670227249503832650433950735e282,  0 },
    { 160, 4.71472363599206132240694321176e284,  0 },
    { 161, 7.59070505394721872907517857094e286,  0 },
    { 162, 1.22969421873944943411017892849e289,  0 },
    { 163, 2.00440157654530257759959165344e291,  0 },
    { 164, 3.28721858553429622726333031164e293,  0 },
    { 165, 5.42391066613158877498449501421e295,  0 },
    { 166, 9.00369170577843736647426172359e297,  0 },
    { 167, 1.50361651486499904020120170784e300,  0 },
    { 168, 2.52607574497319838753801886917e302,  0 },
    { 169, 4.26906800900470527493925188890e304,  0 },
    { 170, 7.25741561530799896739672821113e306,  0 },

    /*
    { 171, 1.24101807021766782342484052410e309,  0 },
    { 172, 2.13455108077438865629072570146e311,  0 },
    { 173, 3.69277336973969237538295546352e313,  0 },
    { 174, 6.42542566334706473316634250653e315,  0 },
    { 175, 1.12444949108573632830410993864e318,  0 },
    { 176, 1.97903110431089593781523349201e320,  0 },
    { 177, 3.50288505463028580993296328086e322,  0 },
    { 178, 6.23513539724190874168067463993e324,  0 },
    { 179, 1.11608923610630166476084076055e327,  0 },
    { 180, 2.00896062499134299656951336898e329,  0 },
    { 181, 3.63621873123433082379081919786e331,  0 },
    { 182, 6.61791809084648209929929094011e333,  0 },
    { 183, 1.21107901062490622417177024204e336,  0 },
    { 184, 2.22838537954982745247605724535e338,  0 },
    { 185, 4.12251295216718078708070590390e340,  0 },
    { 186, 7.66787409103095626397011298130e342,  0 },
    { 187, 1.43389245502278882136241112750e345,  0 },
    { 188, 2.69571781544284298416133291969e347,  0 },
    { 189, 5.09490667118697324006491921822e349,  0 },
    { 190, 9.68032267525524915612334651460e351,  0 },
    { 191, 1.84894163097375258881955918429e354,  0 },
    { 192, 3.54996793146960497053355363384e356,  0 },
    { 193, 6.85143810773633759312975851330e358,  0 },
    { 194, 1.32917899290084949306717315158e361,  0 },
    { 195, 2.59189903615665651148098764559e363,  0 },
    { 196, 5.08012211086704676250273578535e365,  0 },
    { 197, 1.00078405584080821221303894971e368,  0 },
    { 198, 1.98155243056480026018181712043e370,  0 },
    { 199, 3.94328933682395251776181606966e372,  0 },
    { 200, 7.88657867364790503552363213932e374,  0 }
    */
};

#define DOUB_FACT_TABLE_MAX  297
#define DOUB_FACT_TABLE_SIZE (DOUB_FACT_TABLE_MAX+1)
static struct {int n; double f; long i; } doub_fact_table[DOUB_FACT_TABLE_SIZE] = {
  { 0,  1.000000000000000000000000000,    1L    },
  { 1,  1.000000000000000000000000000,    1L    },
  { 2,  2.000000000000000000000000000,    2L    },
  { 3,  3.000000000000000000000000000,    3L    },
  { 4,  8.000000000000000000000000000,    8L    },
  { 5,  15.00000000000000000000000000,    15L   },
  { 6,  48.00000000000000000000000000,    48L	},
  { 7,  105.0000000000000000000000000,    105L  },
  { 8,  384.0000000000000000000000000,    384L  },
  { 9,  945.0000000000000000000000000,    945L  },
  { 10, 3840.000000000000000000000000,    3840L   },
  { 11, 10395.00000000000000000000000,    10395L  },
  { 12, 46080.00000000000000000000000,       46080L       },
  { 13, 135135.0000000000000000000000,       135135L      },
  { 14, 645120.00000000000000000000000,      645120L      },
  { 15, 2.02702500000000000000000000000e6,   2027025L     },
  { 16, 1.03219200000000000000000000000e7,   10321920L    },
  { 17, 3.4459425000000000000000000000e7,    34459425L    },
  { 18, 1.85794560000000000000000000000e8,   185794560L   },
  { 19, 6.5472907500000000000000000000e8,            0 },
  { 20, 3.7158912000000000000000000000e9,            0 },
  { 21, 1.37493105750000000000000000000e10,          0 },
  { 22, 8.1749606400000000000000000000e10,           0 },
  { 23, 3.1623414322500000000000000000e11,           0 },
  { 24, 1.96199055360000000000000000000e12,          0 },
  { 25, 7.9058535806250000000000000000e12,           0 },
  { 26, 5.1011754393600000000000000000e13,           0 },
  { 27, 2.13458046676875000000000000000e14,          0 },
  { 28, 1.42832912302080000000000000000e15,          0 },
  { 29, 6.1902833536293750000000000000e15,           0 },
  { 30, 4.2849873690624000000000000000e16,           0 },
  { 31, 1.91898783962510625000000000000e17,          0 },
  { 32, 1.37119595809996800000000000000e18,          0 },
  { 33, 6.3326598707628506250000000000e18,           0 },
  { 34, 4.6620662575398912000000000000e19,           0 },
  { 35, 2.21643095476699771875000000000e20,          0 },
  { 36, 1.67834385271436083200000000000e21,          0 },
  { 37, 8.2007945326378915593750000000e21,           0 },
  { 38, 6.3777066403145711616000000000e22,           0 },
  { 39, 3.1983098677287777081562500000e23,           0 },
  { 40, 2.55108265612582846464000000000e24,          0 },
  { 41, 1.31130704576879886034406250000e25,          0 },
  { 42, 1.07145471557284795514880000000e26,          0 },
  { 43, 5.6386202968058350994794687500e26,           0 },
  { 44, 4.7144007485205310026547200000e27,           0 },
  { 45, 2.53737913356262579476576093750e28,          0 },
  { 46, 2.16862434431944426122117120000e29,          0 },
  { 47, 1.19256819277443412353990764062e30,          0 },
  { 48, 1.04093968527333324538616217600e31,          0 },
  { 49, 5.8435841445947272053455474391e31,           0 },
  { 50, 5.2046984263666662269308108800e32,           0 },
  { 51, 2.98022791374331087472622919392e33,          0 },
  { 52, 2.70644318171066643800402165760e34,          0 },
  { 53, 1.57952079428395476360490147278e35,          0 },
  { 54, 1.46147931812375987652217169510e36,          0 },
  { 55, 8.6873643685617511998269581003e36,           0 },
  { 56, 8.1842841814930553085241614926e37,           0 },
  { 57, 4.9517976900801981839013661172e38,           0 },
  { 58, 4.7468848252659720789440136657e39,           0 },
  { 59, 2.92156063714731692850180600912e40,       0 },
  { 60, 2.84813089515958324736640819942e41,       0 },
  { 61, 1.78215198865986332638610166557e42,       0 },
  { 62, 1.76584115499894161336717308364e43,       0 },
  { 63, 1.12275575285571389562324404931e44,       0 },
  { 64, 1.13013833919932263255499077353e45,       0 },
  { 65, 7.2979123935621403215510863205e45,        0 },
  { 66, 7.4589130387155293748629391053e46,        0 },
  { 67, 4.8896013036866340154392278347e47,        0 },
  { 68, 5.0720608663265599749067985916e48,        0 },
  { 69, 3.3738248995437774706530672060e49,        0 },
  { 70, 3.5504426064285919824347590141e50,        0 },
  { 71, 2.39541567867608200416367771623e51,       0 },
  { 72, 2.55631867662858622735302649017e52,       0 },
  { 73, 1.74865344543353986303948473285e53,       0 },
  { 74, 1.89167582070515380824123960272e54,       0 },
  { 75, 1.31149008407515489727961354964e55,       0 },
  { 76, 1.43767362373591689426334209807e56,       0 },
  { 77, 1.00984736473786927090530243322e57,       0 },
  { 78, 1.12138542651401517752540683649e58,       0 },
  { 79, 7.9777941814291672401518892225e58,        0 },
  { 80, 8.9710834121121214202032546920e59,        0 },
  { 81, 6.4620132869576254645230302702e60,        0 },
  { 82, 7.3562883979319395645666688474e61,        0 },
  { 83, 5.3634710281748291355541151243e62,        0 },
  { 84, 6.1792822542628292342360018318e63,        0 },
  { 85, 4.5589503739486047652209978556e64,        0 },
  { 86, 5.3141827386660331414429615754e65,        0 },
  { 87, 3.9662868253352861457422681344e66,        0 },
  { 88, 4.6764808100261091644698061863e67,        0 },
  { 89, 3.5299952745484046697106186396e68,        0 },
  { 90, 4.2088327290234982480228255677e69,        0 },
  { 91, 3.2122956998390482494366629620e70,        0 },
  { 92, 3.8721261107016183881809995223e71,        0 },
  { 93, 2.98743500085031487197609655470e72,       0 },
  { 94, 3.6397985440595212848901395509e73,        0 },
  { 95, 2.83806325080779912837729172696e74,       0 },
  { 96, 3.4942066022971404334945339689e75,        0 },
  { 97, 2.75292135328356515452597297515e76,       0 },
  { 98, 3.4243224702511976248246432895e77,        0 },
  { 99, 2.72539213975072950298071324540e78,       0 },
  { 100, 3.4243224702511976248246432895e79,       0 },
  { 101, 2.75264606114823679801052037785e80,      0 },
  { 102, 3.4928089196562215773211361553e81,       0 },
  { 103, 2.83522544298268390195083598919e82,      0 },
  { 104, 3.6325212764424704404139816015e83,       0 },
  { 105, 2.97698671513181809704837778865e84,      0 },
  { 106, 3.8504725530290186668388204976e85,       0 },
  { 107, 3.1853757851910453638417642339e86,       0 },
  { 108, 4.1585103572713401601859261374e87,       0 },
  { 109, 3.4720596058582394465875230149e88,       0 },
  { 110, 4.5743613929984741762045187512e89,       0 },
  { 111, 3.8539861625026457857121505465e90,       0 },
  { 112, 5.1232847601582910773490610013e91,       0 },
  { 113, 4.3550043636279897378547301176e92,       0 },
  { 114, 5.8405446265804518281779295415e93,       0 },
  { 115, 5.0082550181721881985329396352e94,       0 },
  { 116, 6.7750317668333241206863982681e95,       0 },
  { 117, 5.8596583712614601922835393732e96,       0 },
  { 118, 7.9945374848633224624099499564e97,       0 },
  { 119, 6.9729934618011376288174118541e98,       0 },
  { 120, 9.5934449818359869548919399477e99,       0 },
  { 121, 8.4373220887793765308690683435e100,      0 },
  { 122, 1.17040028778399040849681667362e102,       0 },
  { 123, 1.03779061691986331329689540625e103,       0 },
  { 124, 1.45129635685214810653605267528e104,       0 },
  { 125, 1.29723827114982914162111925781e105,       0 },
  { 126, 1.82863340963370661423542637086e106,       0 },
  { 127, 1.64749260436028300985882145742e107,       0 },
  { 128, 2.34065076433114446622134575470e108,       0 },
  { 129, 2.12526545962476508271787968008e109,       0 },
  { 130, 3.04284599363048780608774948111e110,       0 },
  { 131, 2.78409775210844225836042238090e111,       0 },
  { 132, 4.0165567115922439040358293151e112,        0 },
  { 133, 3.7028500103042282036193617666e113,        0 },
  { 134, 5.3821859935336068314080112822e114,        0 },
  { 135, 4.9988475139107080748861383849e115,        0 },
  { 136, 7.3197729512057052907148953438e116,        0 },
  { 137, 6.8484210940576700625940095873e117,        0 },
  { 138, 1.01012866726638733011865555744e119,       0 },
  { 139, 9.5193053207401613870056733264e119,        0 },
  { 140, 1.41418013417294226216611778042e121,       0 },
  { 141, 1.34222205022436275556779993902e122,       0 },
  { 142, 2.00813579052557801227588724819e123,       0 },
  { 143, 1.91937753182083874046195391280e124,       0 },
  { 144, 2.89171553835683233767727763739e125,       0 },
  { 145, 2.78309742114021617366983317355e126,       0 },
  { 146, 4.2219046860009752130088253506e127,        0 },
  { 147, 4.0911532090761177752946547651e128,        0 },
  { 148, 6.2484189352814433152530615189e129,        0 },
  { 149, 6.0958182815234154851890356000e130,        0 },
  { 150, 9.3726284029221649728795922783e131,        0 },
  { 151, 9.2046856051003573826354437561e132,        0 },
  { 152, 1.42463951724416907587769802630e134,       0 },
  { 153, 1.40831689758035467954322289468e135,       0 },
  { 154, 2.19394485655602037685165496051e136,       0 },
  { 155, 2.18289119124954975329199548675e137,       0 },
  { 156, 3.4225539762273917878885817384e138,        0 },
  { 157, 3.4271391702617931126684329142e139,        0 },
  { 158, 5.4076352824392790248639591467e140,        0 },
  { 159, 5.4491512807162510491428083336e141,        0 },
  { 160, 8.6522164519028464397823346347e142,        0 },
  { 161, 8.7731335619531641891199214170e143,        0 },
  { 162, 1.40165906520826112324473821082e145,       0 },
  { 163, 1.43002077059836576282654719098e146,       0 },
  { 164, 2.29872086694154824212137066574e147,       0 },
  { 165, 2.35953427148730350866380286512e148,       0 },
  { 166, 3.8158766391229700819214753051e149,        0 },
  { 167, 3.9404222333837968594685507847e150,        0 },
  { 168, 6.4106727537265897376280785126e151,        0 },
  { 169, 6.6593135744186166925018508262e152,        0 },
  { 170, 1.08981436813352025539677334714e154,       0 },
  { 171, 1.13874262122558345441781649128e155,       0 },
  { 172, 1.87448071318965483928245015709e156,       0 },
  { 173, 1.97002473472025937614282252992e157,       0 },
  { 174, 3.2615964409499994203514632733e158,        0 },
  { 175, 3.4475432857604539082499394274e159,        0 },
  { 176, 5.7404097360719989798185753611e160,        0 },
  { 177, 6.1021516157960034176023927864e161,        0 },
  { 178, 1.02179293302081581840770641427e163,       0 },
  { 179, 1.09228513922748461175082830877e164,       0 },
  { 180, 1.83922727943746847313387154568e165,       0 },
  { 181, 1.97703610200174714726899923887e166,       0 },
  { 182, 3.3473936485761926211036462131e167,        0 },
  { 183, 3.6179760666631972795022686071e168,        0 },
  { 184, 6.1592043133801944228307090322e169,        0 },
  { 185, 6.6932557233269149670791969232e170,        0 },
  { 186, 1.14561200228871616264651187999e172,       0 },
  { 187, 1.25163882026213309884380982464e173,       0 },
  { 188, 2.15375056430278638577544233437e174,       0 },
  { 189, 2.36559737029543155681480056857e175,       0 },
  { 190, 4.0921260721752941329733404353e176,        0 },
  { 191, 4.5182909772642742735162690860e177,        0 },
  { 192, 7.8568820585765647353088136358e178,        0 },
  { 193, 8.7203015861200493478863993359e179,        0 },
  { 194, 1.52423511936385355864990984535e181,       0 },
  { 195, 1.70045880929340962283784787050e182,       0 },
  { 196, 2.98750083395315297495382329688e183,       0 },
  { 197, 3.3499038543080169569905603049e184,        0 },
  { 198, 5.9152516512272428904085701278e185,        0 },
  { 199, 6.6663086700729537444112150067e186,        0 },
  { 200, 1.18305033024544857808171402556e188,       0 },
  { 201, 1.33992804268466370262665421635e189,       0 },
  { 202, 2.38976166709580612772506233164e190,       0 },
  { 203, 2.72005392664986731633210805920e191,       0 },
  { 204, 4.8751138008754445005591271565e192,        0 },
  { 205, 5.5761105496322279984808215214e193,        0 },
  { 206, 1.00427344298034156711518019425e195,       0 },
  { 207, 1.15425488377387119568553005492e196,       0 },
  { 208, 2.08888876139911045959957480403e197,       0 },
  { 209, 2.41239270708739079898275781478e198,       0 },
  { 210, 4.3866663989381319651591070885e199,        0 },
  { 211, 5.0901486119543945858536189892e200,        0 },
  { 212, 9.2997327657488397661373070276e201,        0 },
  { 213, 1.08420165434628604678682084470e203,       0 },
  { 214, 1.99014281187025170995338370390e204,       0 },
  { 215, 2.33103355684451500059166481610e205,       0 },
  { 216, 4.2987084736397436934993088004e206,        0 },
  { 217, 5.0583428183525975512839126509e207,        0 },
  { 218, 9.3711844725346412518284931849e208,        0 },
  { 219, 1.10777707721921886373117687056e210,       0 },
  { 220, 2.06166058395762107540226850068e211,       0 },
  { 221, 2.44818734065447368884590088393e212,       0 },
  { 222, 4.5768864963859187873930360715e213,        0 },
  { 223, 5.4594577696594763261263589712e214,        0 },
  { 224, 1.02522257519044580837604008002e216,       0 },
  { 225, 1.22837799817338217337843076851e217,       0 },
  { 226, 2.31700301993040752692985058084e218,       0 },
  { 227, 2.78841805585357753356903784452e219,       0 },
  { 228, 5.2827668854413291614000593243e220,        0 },
  { 229, 6.3854773479046925518730966640e221,        0 },
  { 230, 1.21503638365150570712201364459e223,       0 },
  { 231, 1.47504526736598397948268532937e224,       0 },
  { 232, 2.81888441007149324052307165546e225,       0 },
  { 233, 3.4368554729627426721946568174e226,        0 },
  { 234, 6.5961895195672941828239876738e227,        0 },
  { 235, 8.0766103614624452796574435210e228,        0 },
  { 236, 1.55670072661788142714646109101e230,       0 },
  { 237, 1.91415665566659953127881411447e231,       0 },
  { 238, 3.7049477293505577966085773966e232,        0 },
  { 239, 4.5748344070431728797563657336e233,        0 },
  { 240, 8.8918745504413387118605857518e234,        0 },
  { 241, 1.10253509209740466402128414180e236,       0 },
  { 242, 2.15183364120680396827026175195e237,       0 },
  { 243, 2.67916027379669333357172046456e238,       0 },
  { 244, 5.2504740845446016825794386748e239,        0 },
  { 245, 6.5639426708018986672507151382e240,        0 },
  { 246, 1.29161662479797201391454191399e242,       0 },
  { 247, 1.62129383968806897081092663913e243,       0 },
  { 248, 3.2032092294989705945080639467e244,        0 },
  { 249, 4.0370216608232917373192073314e245,        0 },
  { 250, 8.0080230737474264862701598667e246,        0 },
  { 251, 1.01329243686664622606712104019e248,       0 },
  { 252, 2.01802181458435147454008028642e249,       0 },
  { 253, 2.56362986527261495194981623168e250,       0 },
  { 254, 5.1257754090442527453318039275e251,        0 },
  { 255, 6.5372561564451681274720313908e252,        0 },
  { 256, 1.31219850471532870280494180544e254,       0 },
  { 257, 1.68007483220640820876031206743e255,       0 },
  { 258, 3.3854721421655480532367498580e256,        0 },
  { 259, 4.3513938154145972606892082546e257,        0 },
  { 260, 8.8022275696304249384155496309e258,        0 },
  { 261, 1.13571378582320988503988335446e260,       0 },
  { 262, 2.30618362324317133386487400329e261,       0 },
  { 263, 2.98692725671504199765489322224e262,       0 },
  { 264, 6.0883247653619723214032673687e263,        0 },
  { 265, 7.9153572302948612937854670389e264,        0 },
  { 266, 1.61949438758628463749326912007e266,       0 },
  { 267, 2.11340038048872796544071969939e267,       0 },
  { 268, 4.3402449587312428284819612418e268,        0 },
  { 269, 5.6850470235146782270355359914e269,        0 },
  { 270, 1.17186613885743556369012953528e271,       0 },
  { 271, 1.54064774337247779952663025366e272,       0 },
  { 272, 3.1874758976922247332371523360e273,        0 },
  { 273, 4.2059683394068643927077005925e274,        0 },
  { 274, 8.7336839596766957690697974006e275,        0 },
  { 275, 1.15664129333688770799461766294e277,       0 },
  { 276, 2.41049677287076803226326408256e278,       0 },
  { 277, 3.2038963825431789511450909263e279,        0 },
  { 278, 6.7011810285807351296918741495e280,        0 },
  { 279, 8.9388709072954692736948036845e281,        0 },
  { 280, 1.87633068800260583631372476186e283,       0 },
  { 281, 2.51182272495002686590823983534e284,       0 },
  { 282, 5.2912525401673484584047038284e285,        0 },
  { 283, 7.1084583116085760305203187340e286,        0 },
  { 284, 1.50271572140752696218693588728e288,       0 },
  { 285, 2.02591061880844416869829083919e289,       0 },
  { 286, 4.2977669632255271118546366376e290,        0 },
  { 287, 5.8143634759802347641640947085e291,        0 },
  { 288, 1.23775688540895180821413535163e293,       0 },
  { 289, 1.68035104455828784684342337075e294,       0 },
  { 290, 3.5894949676859602438209925197e295,        0 },
  { 291, 4.8898215396646176343143620089e296,        0 },
  { 292, 1.04813253056430039119572981576e298,       0 },
  { 293, 1.43271771112173296685410806860e299,       0 },
  { 294, 3.08150963985904315011544565835e300,       0 },
  { 295, 4.2265172478091122522196188024e301,        0 },
  { 296, 9.1212685339827677243417191487e302,        0 },
  { 297, 1.25527562259930633890922678431e304,       0 },
  /*
  { 298, 2.71813802312686478185383230631e305,       0 },
  { 299, 3.7532741115719259533385880851e306,        0 },
  { 300, 8.1544140693805943455614969189e307,  }
  */
};

/* coefficients for gamma=7, kmax=8  Lanczos method */
static double lanczos_7_c[9] = {
  0.99999999999980993227684700473478,
  676.520368121885098567009190444019,
 -1259.13921672240287047156078755283,
  771.3234287776530788486528258894,
 -176.61502916214059906584551354,
  12.507343278686904814458936853,
 -0.13857109526572011689554707,
  9.984369578019570859563e-6,
  1.50563273514931155834e-7
};

/* complex version of Lanczos method; this is not safe for export
 * since it becomes bad in the left half-plane
 */
static void lngamma_lanczos_complex(double zr, double zi, double * yr, double * yi)
{
  int k;
  double log1_r,    log1_i;
  double logAg_r,   logAg_i;
  double Ag_r, Ag_i;

  zr -= 1.; /* Lanczos writes z! instead of Gamma(z) */

  Ag_r = lanczos_7_c[0];
  Ag_i = 0.;
  for(k=1; k<=8; k++) {
    double R = zr + k;
    double I = zi;
    double a = lanczos_7_c[k] / (R*R + I*I);
    Ag_r +=  a * R;
    Ag_i -=  a * I;
  }

  gsl_sf_complex_log_impl(zr + 7.5, zi, &log1_r,  &log1_i);
  gsl_sf_complex_log_impl(Ag_r, Ag_i,   &logAg_r, &logAg_i);

  /* (z+0.5)*log(z+7.5) - (z+7.5) + LogRootTwoPi_ + log(Ag(z)) */
  *yr = (zr+0.5)*log1_r - zi*log1_i - (zr+7.5) + LogRootTwoPi_ + logAg_r;
  *yi = zi*log1_r + (zr+0.5)*log1_i - zi + logAg_i;
  gsl_sf_angle_restrict_symm_impl(yi, 1.e-12);
}


/*-*-*-*-*-*-*-*-*-*-*-* (semi)Private Implementations *-*-*-*-*-*-*-*-*-*-*-*/

/* Lanczos with gamma=7, truncated at 1/(z+8) 
 * [J. SIAM Numer. Anal, Ser. B, 1 (1964) 86]
 */
int gsl_sf_lngamma_impl(double x, double * result)
{
  if(x <= 0.0) {
    return GSL_EDOM;
  }
  else {
    int k;
    double Ag;

    x -= 1.; /* Lanczos writes z! instead of Gamma(z) */
  
    Ag = lanczos_7_c[0];
    for(k=1; k<=8; k++) { Ag += lanczos_7_c[k]/(x+k); }

    *result =  (x+0.5)*log(x+7.5) - (x+7.5) + LogRootTwoPi_ + log(Ag);
    return GSL_SUCCESS;
  }
}

int gsl_sf_lngamma_complex_impl(double zr, double zi, double * lnr, double * arg)
{
  if(zr <= 0.5) {
    /* transform to right half plane using reflection;
     * in fact we can do a little better by stopping at 1/2
     */
    int status;
    double x = 1.-zr;
    double y = -zi;
    double a, b;
    double lnsin_r, lnsin_i;
    
    lngamma_lanczos_complex(x, y, &a, &b);
    status = gsl_sf_complex_logsin_impl(M_PI*zr, M_PI*zi, &lnsin_r, &lnsin_i);
    
    if(status == GSL_SUCCESS) {
      *lnr = LogPi_ - lnsin_r - a;
      *arg =        - lnsin_i - b;
      gsl_sf_angle_restrict_symm_impl(arg, 10.*GSL_MACH_EPS);
      return GSL_SUCCESS;
    }
    else {
      return GSL_EDOM;
    }
  }
  else {
    /* otherwise plain vanilla Lanczos */
    lngamma_lanczos_complex(zr, zi, lnr, arg);
    return GSL_SUCCESS;
  }
}

int gsl_sf_fact_impl(const unsigned int n, double * result)
{
  if(n <= FACT_TABLE_MAX){
    *result = fact_table[n].f;
    return GSL_SUCCESS;
  }
  else {
    return GSL_EOVRFLW;
  }
}

int gsl_sf_doublefact_impl(const unsigned int n, double * result)
{
  if(n <= DOUB_FACT_TABLE_MAX){
    *result = doub_fact_table[n].f;
    return GSL_SUCCESS;
  }
  else {
    return GSL_EOVRFLW;
  }
}
int gsl_sf_lnfact_impl(const unsigned int n, double * result)
{
  if(n <= FACT_TABLE_MAX){
    *result = log(fact_table[n].f);
    return GSL_SUCCESS;
  }
  else {
    gsl_sf_lngamma_impl(n+1.0, result);
    return GSL_SUCCESS;
  }
}

int gsl_sf_lnchoose_impl(unsigned int n, unsigned int m, double * result)
{
  double nf, mf, nmmf;

  if(m > n) return GSL_EDOM;

  if(m*2 > n) m = n-m;
  gsl_sf_lnfact_impl(n, &nf);
  gsl_sf_lnfact_impl(m, &mf);
  gsl_sf_lnfact_impl(n-m, &nmmf);
  *result = nf - mf - nmmf;
  return GSL_SUCCESS;
}

int gsl_sf_choose_impl(unsigned int n, unsigned int m, double * result)
{
  double nf, mf, nmmf;
  double ln_result;

  if(m > n) return GSL_EDOM;

  if(m*2 > n) m = n-m;
  gsl_sf_lnfact_impl(n, &nf);
  gsl_sf_lnfact_impl(m, &mf);
  gsl_sf_lnfact_impl(n-m, &nmmf);
  ln_result = nf - mf - nmmf;
  
  if(ln_result < GSL_LOG_DBL_MAX) {
    *result = exp(ln_result);
    return GSL_SUCCESS;
  }
  else {
    return GSL_EOVRFLW;
  }
}


/*-*-*-*-*-*-*-*-*-*-*-* Functions w/ Error Handling *-*-*-*-*-*-*-*-*-*-*-*/

int gsl_sf_fact_e(const unsigned int n, double * result)
{
  int status = gsl_sf_fact_impl(n, result);
  if(status != GSL_SUCCESS) {
    GSL_ERROR("gsl_sf_fact_e", status);
  }
  return status;
}

int gsl_sf_lnfact_e(const unsigned int n, double * result)
{
  int status = gsl_sf_lnfact_impl(n, result);
  if(status != GSL_SUCCESS) {
    GSL_ERROR("gsl_sf_lnfact_e", status);
  }
  return status;
}

int gsl_sf_doublefact_e(const unsigned int n, double * result)
{
  int status = gsl_sf_doublefact_impl(n, result);
  if(status != GSL_SUCCESS) {
    GSL_ERROR("gsl_sf_doublefact_e", status);
  }
  return status;
}

int gsl_sf_lngamma_e(const double x, double * result)
{
  int status = gsl_sf_lngamma_impl(x, result);
  if(status != GSL_SUCCESS) {
    GSL_ERROR("gsl_sf_lngamma_e", status);
  }
  return status;
}

int gsl_sf_lngamma_complex_e(double zr, double zi, double * lnr, double * arg)
{
  int status = gsl_sf_lngamma_complex_impl(zr, zi, lnr, arg);
  if(status != GSL_SUCCESS) {;
    GSL_ERROR("gsl_sf_lngamma_complex_e", status);
  }
  return status;
}

int gsl_sf_choose_e(unsigned int n, unsigned int m, double * r)
{
  int status = gsl_sf_choose_impl(n, m, r);
  if(status != GSL_SUCCESS) {;
    GSL_ERROR("gsl_sf_choose_e", status);
  }
  return status;
}

int gsl_sf_lnchoose_e(unsigned int n, unsigned int m, double * r)
{
  int status = gsl_sf_lnchoose_impl(n, m, r);
  if(status != GSL_SUCCESS) {;
    GSL_ERROR("gsl_sf_lnchoose_e", status);
  }
  return status;
}


/*-*-*-*-*-*-*-*-*-*-*-* Functions w/ Natural Prototypes *-*-*-*-*-*-*-*-*-*-*-*/

double gsl_sf_lngamma(const double x)
{
  double y;
  int status = gsl_sf_lngamma_impl(x, &y);
  if(status != GSL_SUCCESS) {
    GSL_WARNING("gsl_sf_lngamma", status);
  }
  return y;
}

double gsl_sf_lnfact(const unsigned int n)
{
  double y;
  int status = gsl_sf_lnfact_impl(n, &y);
  if(status != GSL_SUCCESS) {
    GSL_WARNING("gsl_sf_lnfact", status);
  }
  return y;
}

double gsl_sf_lnchoose(unsigned int n, unsigned int m)
{
  double y;
  int status = gsl_sf_lnchoose_impl(n, m, &y);
  if(status != GSL_SUCCESS) {
    GSL_WARNING("gsl_sf_lnchoose", status);
  }
  return y;
}

double gsl_sf_choose(unsigned int n, unsigned int m)
{
  double y;
  int status = gsl_sf_choose_impl(n, m, &y);
  if(status != GSL_SUCCESS) {
    GSL_WARNING("gsl_sf_choose", status);
  }
  return y;
}
