#include "../src/dpc.h"
#include "../src/file_define_catapult.h"
#include <ac_int.h>
#include <mc_scverify.h>

CCS_MAIN(int argc, char** argv)
{
    top_register topParam;
    dpc_register dpc_param;
    ac_channel<uint12> src;
    ac_channel<uint12> dst;
    int x;

    printf("\tTest for ISP dpc module!\n");
    memset(&topParam, 0, sizeof(top_register));
    memset(&dpc_param, 0, sizeof(dpc_register));

    const char* config_file = "../config/xkISP.cfg";

    char buf[100] = "";
    FILE* fp_config = fopen((const char*)config_file, "r");
    char *p, *q;
    char key[100], value[100];
    int output_yuvpattern = 0; //0:444 1:422 2:420
    int Noise_Mode;
    int Img_Format;
    float rawdns_sigma;
    int noise_es_enable;
    int lsc_config = 0;
    float gtm_gamma = 0.0;

    if(fp_config == NULL)
    {
        printf("\t Warning: no configuration file!\n");
        printf("\t Will use default initial values!\n");
    }
    else
    {
        while (fgets(buf, 100, fp_config))
        {
            p = strchr(buf, '=');
            q = strchr(buf, '\n');
            if (p != NULL && q != NULL)
            {
                *q = '\0';
                strncpy(key, buf, p - buf);
                strcpy(value, p + 1);

                if(strstr(key, "frame_width"))
                {
                    topParam.frameWidth = atoi(value);
                    printf("frame_width = %d\n", topParam.frameWidth);
                    continue;
                }

                if(strstr(key, "frame_height"))
                {
                    topParam.frameHeight = atoi(value);
                    printf("frame_height = %d\n", topParam.frameHeight);
                    continue;
                }

                if(strstr(key, "image_pattern"))
                {
                    topParam.imgPattern = atoi(value);
                    printf("image_pattern = %d\n", topParam.imgPattern);
                    continue;
                }

                if(strstr(key, "blc"))
                {
                    topParam.blc = atoi(value);
                    printf("blc = %d\n", topParam.blc);
                    continue;
                }

                if(strstr(key, "dpc_enable"))
                {
                   dpc_param.eb = atoi(value);
                    printf("dpc_enable = %d\n",dpc_param.eb);
                    continue;
                }

                if(strstr(key, "dpc_threshold_w"))
                {
                   dpc_param.th_w = atoi(value);
                    printf("dpc_threshod_w = %d\n",dpc_param.th_w);
                    continue;
                }

                if(strstr(key, "dpc_threshold_b"))
                {
                   dpc_param.th_b = atoi(value);
                    printf("dpc_threshod_b = %d\n",dpc_param.th_b);
                    continue;
                }
            }
        }
    }

    uint16_t frameIn;
    uint16_t frameGolden;
    uint16_t frameOut;

    //In
    FILE *fp_r1 = fopen(DPC_SRC1, "r");
    if(!fp_r1){
        printf("Can not input file!\n");
    }

    for (x = 0; x < topParam.frameWidth*topParam.frameHeight; x++) {
        fread(&frameIn, sizeof(uint16_t), 1, fp_r1);
        uint12 srcdata = (uint12)frameIn;
        src.write(srcdata);
    }
    printf("\tInit done!\n");

    //Golden
    FILE *fp_g1 = fopen(DPC_GOLDEN1, "r");
    if(!fp_g1){
        printf("Can not open golden file!\n");
    }

    printf("\tEnvironment set up!\n");

    //Execution
    CCS_DESIGN(dpc) (topParam, dpc_param, src, dst);
    printf("\tExecution completed!\n");

    //Out
    FILE *fp_w1 = fopen(DPC_DST1, "w");
    if(!fp_w1){
        printf("\tCan not open write back file!\n");
    }

    for (x = 0; x < topParam.frameWidth*topParam.frameHeight; x++) {
        fread(&frameGolden, sizeof(uint16_t), 1, fp_g1);
        uint12 dstdata;
        dstdata = dst.read();
        frameOut = dstdata;
        fwrite(&frameOut, sizeof(uint16_t), 1, fp_w1);
        if(frameGolden != frameOut) {
            printf("\t\tFirst mismatch in pixel %d!\n", x);
            printf("\t\tGolden = %d, result = %d!\n", frameGolden, frameOut);
            exit(0);
        }
        frameOut = 0;
        frameGolden = 0;
    }

    printf("\tTest passed!\n");

    fclose(fp_r1);
    fclose(fp_g1);
    fclose(fp_w1);

    return 0;
}
