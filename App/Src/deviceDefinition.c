#include "app.h"
#include "DD_Gene.h"
#include "SystemTaskManager.h"

/*Address Definition*/
#if DD_NUM_OF_MD
/*MD Definition*/
DD_MDHand_t g_md_h[DD_NUM_OF_MD] = {
  { .add  = 0x10, /* address */
    .duty = 0, /* default duty */
    .mode = D_MMOD_FREE, /* mode */
  },
  { .add  = 0x11, /* address */
    .duty = 0, /* default duty */
    .mode = D_MMOD_FREE, /* mode */
  },
  { .add  = 0x12, /* address */
    .duty = 0, /* default duty */
    .mode = D_MMOD_FREE, /* mode */
  },
  { .add  = 0x13, /* address */
    .duty = 0, /* default duty */
    .mode = D_MMOD_FREE, /* mode */
  },
};
#endif
#if DD_NUM_OF_LD
/*LD Definition*/
DD_LDHand_t g_ld_h[DD_NUM_OF_LD] = {
  { .add  = 0x70, /* address */
    .mode = {0,0,0,0,0,0,0,0}, /* mode */
  },
};
#endif
#if DD_NUM_OF_AB
/*AB Definition*/
DD_ABHand_t g_ab_h[DD_NUM_OF_AB] = {
  { .add = 0x20, /* address */
    .dat = 0x00, /* data */
  },
};
#endif
#if DD_NUM_OF_SV
DD_SV_t g_sv_h = {
  .i2cadd = 0x40,/*address*/
  .val = {
    0,0,0,0
  }
};
#endif
#if DD_NUM_OF_SS
DD_SSHand_t g_ss_h[DD_NUM_OF_SS] = {
  { .add = 0x18,    /*I2C address*/
    .data_size = 8, /*最大8バイト 無駄なく宣言する*/
    .data = {0,0,0,0,0,0,0,0},
    .type = D_STYP_ENCODER,
  },
  { .add = 0x36,    /*I2C address*/
    .data_size = 2, /*最大8バイト 無駄なく宣言する*/
    .data = {0,0,0,0,0,0,0,0},
    .type = D_STYP_PHOTOARRAY,
  },
  { .add = 0x38,    /*I2C address*/
    .data_size = 2, /*最大8バイト 無駄なく宣言する*/
    .data = {0,0,0,0,0,0,0,0},
    .type = D_STYP_PHOTOARRAY,
  },
  /* { .add = 0x1c,    /\*I2C address*\/ */
  /*   .data_size = 8, /\*最大8バイト 無駄なく宣言する*\/ */
  /*   .data = {0,0,0,0,0,0,0,0}, */
  /*   .type = D_STYP_ENCODER, */
  /* }, */
  { .add = 0x53,    /*I2C address*/
    .data_size = 2, /*最大8バイト 無駄なく宣言する*/
    .data = {0,0,0,0,0,0,0,0},
    .type = D_STYP_PHOTOARRAY,
  },
  { .add = 0x51,    /*I2C address*/
    .data_size = 2, /*最大8バイト 無駄なく宣言する*/
    .data = {0,0,0,0,0,0,0,0},
    .type = D_STYP_PHOTOARRAY,
  },
  
};
#endif
