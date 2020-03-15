#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "pav_analysis.h"
#include "vad.h"

const float FRAME_TIME = 10.0F; /* in ms. */

/* 
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT", "M"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
} Features;

/* 
 * TODO: Delete and use your own features!
 */

Features compute_features(const float *x, int N) {
  Features feat;
  float fm = 16000;
  feat.zcr = compute_zcr(x,N,fm);
  feat.p = compute_power(x,N);
  feat.am = compute_am(x,N);
  return feat;
}

/* 
 * TODO: Init the values of vad_data
 */

VAD_DATA * vad_open(float rate) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;

  vad_data->power = 0;
  
  vad_data->p_alpha1 = 5; //Alpha para k1

  vad_data->fr_cont = 0; //Contador de los frames .
  
  vad_data->fr_threshold_silence = 5; //Threshold del límite de veces de silence.
  vad_data->fr_threshold_voice = 5;  //Threshold del límite de veces de voice.

  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /*TODO: decide what to do with the last undecided frames*/
  VAD_STATE state = ST_SILENCE;
  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/* 
 * TODO: Implement the Voice Activity Detection 
 * using a Finite State Automata
 */

VAD_STATE vad(VAD_DATA *vad_data, float *x) {

  /* 
   * TODO: You can change this, using your own features,
   * program finite state automaton, define conditions, etc.
   */
  static float power_array[10];
  Features f = compute_features(x, vad_data->frame_length);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */

  switch (vad_data->state) {
  case ST_INIT:
    power_array[vad_data->fr_cont] = vad_data->last_feature;
    vad_data->fr_cont++;
    if(vad_data->fr_cont == 10){
      vad_data->state = ST_SILENCE;
      for(unsigned int i=0; i<10; i++){
        vad_data->power += pow(10, power_array[i]/10);
      }
      vad_data->power = 10*log10(vad_data->power/10);
      vad_data->k0 = vad_data->power;
      vad_data->k1 = vad_data->k0 + vad_data->p_alpha1;

      vad_data->fr_cont = 0;
      vad_data->power =0;
    }    
    break;

  case ST_SILENCE:
    if (f.p > vad_data->k1)
      vad_data->state = ST_MAYBE_VOICE;
    break;

  case ST_VOICE:
    if (f.p < vad_data->k0)
      vad_data->state = ST_MAYBE_SILENCE;
    break;

  case ST_MAYBE_VOICE:
    vad_data->fr_cont++;
    if (vad_data->fr_cont==vad_data->fr_threshold_voice){
      vad_data->state = ST_VOICE;
      vad_data->fr_cont =0;
    }else if(f.p > vad_data->k1)
      vad_data->state = ST_MAYBE_VOICE;
    else{
      vad_data->state = ST_MAYBE_SILENCE;
      vad_data->fr_cont = 0;
    }
    break;
  case ST_MAYBE_SILENCE:
    vad_data->fr_cont++;
    if (vad_data->fr_cont==vad_data->fr_threshold_silence){
      vad_data->state = ST_SILENCE;
      vad_data->fr_cont =0;
    }else if(f.p < vad_data->k1)
      vad_data->state = ST_MAYBE_SILENCE;
    else{
      vad_data->state = ST_MAYBE_VOICE;
      vad_data->fr_cont = 0;
    }
    break;
  case ST_UNDEF:
    break;
  }

  if (vad_data->state == ST_SILENCE || vad_data->state == ST_VOICE){
    //sumar aqui y solo tener un contador.
    return vad_data->state;
  }
  else
    return ST_UNDEF;
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}
