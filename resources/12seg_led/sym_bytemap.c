
uint16_t syms[] = {
0x0000, 0x1400, 0x8002, 0xe0aa, //sp, !, ", #
0xbbaa, 0x1144, 0xe192, 0x0100, //$, %, &, '
0x0014, 0x0041, 0x00ff, 0x00aa, // (, ), *, +
0x1000, 0x0008, 0x0008, 0x0044, //, -, . , /
0x0c00, 0x7788, 0x3f88, 0x9d88, // 1, 2, 3, 4
0xbb88, 0xfb88, 0x1f00, 0xff88, // 5, 6, 7, 8
0xbf88, 0xff44, 0x0000, 0x0000, // 9, 0, :, ;
0x0000, 0x0000, 0x0000, 0x0728, // <, =, >, ?
0x7fa0, 0xdf88, 0xfb8c, 0xf300, // @, A, B, C,
0x3f22, 0xf388, 0xc388, 0xfb08, // D, E, F, G, 
0xdd88, 0x7c00, 0xd194, 0xf100, // H, J, K, L,
0xdd05, 0xdd11, 0xff00, 0xc788, // M, N, O, P,
0xd798, 0xbb88, 0x0322, 0xfd00, // R, S, T, U,
0xc144, 0xdd50, 0x1155, 0x0125, // V, W, X, Y,
0x3344, 0xe300, 0x0011, 0x3e00, // Z, [, \, ],
0x0050, 0x2000, 0x0001, 0xdf88, // ^, _, `, a,
0xfb8c, 0xf300, 0x3f22, 0xf388, // B, C, D, E
0xc388, 0xfb08, 0xdd88, 0x7c00, // F, G, H, J,
0xd194, 0xf100, 0xdd05, 0xdd11, // K, L, M, N
0xff00, 0xc788, 0xd798, 0xbb88, // O, P, R, S
0x0322, 0xfd00, 0xc144, 0xdd50, // T, U, V, W
0x1155, 0x0125, 0x3344, 0x32a2// X, Y, Z, {,
0x0022, 0x212a, 0x8888 // |, }, ~
};


uint16_t digits[] = {
0x0c00, 0x7788, 0x3f88, 0x9d88, // 1, 2, 3, 4
0xbb88, 0xfb88, 0x1f00, 0xff88, // 5, 6, 7, 8
0xbf88, 0xff44,  // 9, 0,
}

uint16_t letter_en[] = {
  0xdf88, 0xfb8c, 0xf300, 0x3f22, //A, B, C, D
  0xf388, 0xc388, 0xfb08, 0xdd88, //E, F, G, H
  0x7c00, 0xd194, 0xf100, 0xdd05, //J, K, L, M
  0xdd11, 0xff00, 0xc788, 0xd798, //N, O, P, R
  0xbb88, 0x0322, 0xfd00, 0xc144, //S, T, U, V
  0xdd50, 0x1155, 0x0125, 0x3344, //W, X, Y, Z
};

uint16_t letter_ru[] = {
 0xdf88, //А
 0xf390, 
 0xf394,
 0xc300,
 0x3c44,
 0xf388,
 0x0077,
 0x3b0c,
 0xdd44,
 0xde44,
 0xd194,
 0x0e42,
 0xdd05,
 0xdd88,
 0xff00,
 0xdf00,
 0xc788,
 0xf300,
 0x0322,
 0x3d09,
 0x87aa,
 0x1155,
 0xf022,
 0x9d88,
 0xec22,
 0xf42a,
 0x392a,
 0x382a,
 0xeda0,
 0x3e88,
 0xdeaa,
 0x0f49
};