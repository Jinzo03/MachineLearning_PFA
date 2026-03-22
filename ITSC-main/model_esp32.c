/*
 * Modele de classification : Random Forest
 * Accuracy sur le jeu de test : 0.9231
 * Nombre de caracteristiques en entree : 45
 * Classes de sortie : 0=Normal, 1=Surcharge, 2=Court-circuit
 */

#include <string.h>
void add_vectors(double *v1, double *v2, int size, double *result) {
    for(int i = 0; i < size; ++i)
        result[i] = v1[i] + v2[i];
}
void mul_vector_number(double *v1, double num, int size, double *result) {
    for(int i = 0; i < size; ++i)
        result[i] = v1[i] * num;
}
void score(double * input, double * output) {
    double var0[3];
    double var1[3];
    double var2[3];
    double var3[3];
    double var4[3];
    double var5[3];
    double var6[3];
    double var7[3];
    double var8[3];
    double var9[3];
    double var10[3];
    if (input[23] <= 0.8683660626411438) {
        if (input[5] <= 0.7889077663421631) {
            if (input[27] <= -1.1362038850784302) {
                memcpy(var10, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[30] <= -0.7537500858306885) {
                    if (input[28] <= 0.004815042018890381) {
                        memcpy(var10, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var10, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    memcpy(var10, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            memcpy(var10, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
        }
    } else {
        memcpy(var10, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
    }
    double var11[3];
    if (input[9] <= 0.9736323058605194) {
        if (input[20] <= 0.8460361063480377) {
            if (input[22] <= -1.1296314001083374) {
                if (input[2] <= -0.5670848041772842) {
                    memcpy(var11, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var11, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                if (input[29] <= 1.8385260105133057) {
                    memcpy(var11, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var11, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            memcpy(var11, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
        }
    } else {
        memcpy(var11, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
    }
    add_vectors(var10, var11, 3, var9);
    double var12[3];
    if (input[24] <= 0.8411249220371246) {
        if (input[1] <= 0.8736865520477295) {
            if (input[43] <= -0.8638548254966736) {
                if (input[31] <= -0.9813724458217621) {
                    memcpy(var12, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[30] <= 0.5624349266290665) {
                        memcpy(var12, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var12, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                memcpy(var12, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            memcpy(var12, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
        }
    } else {
        memcpy(var12, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
    }
    add_vectors(var9, var12, 3, var8);
    double var13[3];
    if (input[5] <= 0.8055863082408905) {
        if (input[28] <= 0.7659242153167725) {
            if (input[21] <= -0.3802322596311569) {
                if (input[0] <= 0.11055322363972664) {
                    if (input[23] <= -0.4369844123721123) {
                        memcpy(var13, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var13, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    memcpy(var13, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                memcpy(var13, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            memcpy(var13, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
        }
    } else {
        memcpy(var13, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
    }
    add_vectors(var8, var13, 3, var7);
    double var14[3];
    if (input[9] <= 0.9579035937786102) {
        if (input[43] <= 0.8217982053756714) {
            if (input[1] <= -0.6396353542804718) {
                if (input[13] <= -1.083045482635498) {
                    memcpy(var14, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var14, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                memcpy(var14, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            memcpy(var14, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
        }
    } else {
        memcpy(var14, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
    }
    add_vectors(var7, var14, 3, var6);
    double var15[3];
    if (input[10] <= 0.8430294692516327) {
        if (input[31] <= 1.0675088167190552) {
            if (input[17] <= -0.7885520756244659) {
                if (input[37] <= 0.8219626247882843) {
                    memcpy(var15, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[39] <= -0.7603360414505005) {
                        memcpy(var15, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var15, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                memcpy(var15, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            memcpy(var15, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
        }
    } else {
        memcpy(var15, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
    }
    add_vectors(var6, var15, 3, var5);
    double var16[3];
    if (input[13] <= 0.8739859163761139) {
        if (input[38] <= 0.8224330246448517) {
            if (input[34] <= 0.7921032309532166) {
                memcpy(var16, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[30] <= 0.3297664374113083) {
                    memcpy(var16, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var16, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            memcpy(var16, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
        }
    } else {
        memcpy(var16, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
    }
    add_vectors(var5, var16, 3, var4);
    double var17[3];
    if (input[43] <= 0.7298822402954102) {
        if (input[4] <= -0.793740838766098) {
            memcpy(var17, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
        } else {
            if (input[34] <= 0.9586537778377533) {
                memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[22] <= 0.3514067232608795) {
                    memcpy(var17, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        }
    } else {
        if (input[37] <= 0.7923189997673035) {
            memcpy(var17, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
        } else {
            if (input[35] <= 0.8986164331436157) {
                memcpy(var17, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
            } else {
                memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        }
    }
    add_vectors(var4, var17, 3, var3);
    double var18[3];
    if (input[23] <= 0.835052877664566) {
        if (input[13] <= 0.8739859163761139) {
            if (input[30] <= -0.7877479791641235) {
                if (input[16] <= 0.003335237503051758) {
                    memcpy(var18, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var18, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                memcpy(var18, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            memcpy(var18, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
        }
    } else {
        memcpy(var18, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
    }
    add_vectors(var3, var18, 3, var2);
    double var19[3];
    if (input[6] <= -0.36759500205516815) {
        if (input[9] <= 0.6911459863185883) {
            memcpy(var19, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        } else {
            memcpy(var19, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
        }
    } else {
        if (input[1] <= -0.7482968866825104) {
            if (input[33] <= -0.2399945706129074) {
                memcpy(var19, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[12] <= 0.36741647124290466) {
                    memcpy(var19, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                } else {
                    memcpy(var19, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            if (input[43] <= -1.132148265838623) {
                memcpy(var19, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            } else {
                memcpy(var19, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
            }
        }
    }
    add_vectors(var2, var19, 3, var1);
    mul_vector_number(var1, 0.1, 3, var0);
    memcpy(output, var0, 3 * sizeof(double));
}
