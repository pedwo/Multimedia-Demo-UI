/* libshcodecs camera/encoder control file */
/* Format carried over from VPU tool */

/********************* Tools related options *********************/
frame_number_to_encode                             = 4294967295;

/* Capture from v4l2 */
input_yuv_path                                     = /dev;
input_yuv_file                                     = video0;
yuv_CbCr_format                                    = 1;

/* Output to stdout */
output_directry                                    = .;
output_stream_file                                 = -;

/******************** Encoder related options ********************/
/* Encode as H.264 */
stream_type                                        = 2;
ref_frame_num                                      = 1;
bitrate                                            = 4000000;
x_pic_size                                         = 1280;
y_pic_size                                         = 720;
frame_rate                                         = 300;
I_vop_interval                                     = 30;

mv_mode                                            = 1;
fcode_forward                                      = 1;
search_mode                                        = 4;
search_time_fixed                                  = 1;
rate_ctrl_skip_enable                              = 0;
rate_ctrl_use_prevquant                            = 1;
rate_ctrl_respect_type                             = 0;
rate_ctrl_intra_thr_changeable                     = 0;
control_bitrate_length                             = 0;
intra_macroblock_refresh_cycle                     = 0;
video_format                                       = 2;
frame_num_resolution                               = 30;
noise_reduction                                    = 0;
reaction_param_coeff                               = 10;
weightedQ_mode                                     = 0;

/* ------------------------ */
/* avcbe_other_options_h264 */
/* ------------------------ */
Ivop_quant_initial_value                           = 30;
Pvop_quant_initial_value                           = 30;
use_dquant                                         = 1;
clip_dquant_next_mb                                = 4;
clip_dquant_frame                                  = 7;
quant_min                                          = 10;
quant_min_Ivop_under_range                         = 4;
quant_max                                          = 40;
rate_ctrl_cpb_skipcheck_enable                     = 1;
rate_ctrl_cpb_Ivop_noskip                          = 1;
rate_ctrl_cpb_remain_zero_skip_enable              = 1;
rate_ctrl_cpb_buffer_unit_size                     = 1000;
rate_ctrl_cpb_buffer_mode                          = 1;
rate_ctrl_cpb_max_size                             = 1152;
rate_ctrl_cpb_offset                               = 20;
rate_ctrl_cpb_offset_rate                          = 50;
intra_thr_1                                        = 0;
intra_thr_2                                        = 5000;
sad_intra_bias                                     = 0;
regularly_inserted_I_type                          = 11;
call_unit:                                         = 1;

use_slice                                          = 0;
slice_size_mb                                      = 0;
slice_size_bit                                     = 0;
slice_type_value_pattern                           = 2;
use_mb_partition                                   = 1;
mb_partition_vector_thr                            = 0;
deblocking_mode                                    = 0;
use_deblocking_filter_control                      = 0;
deblocking_alpha_offset                            = 0;
deblocking_beta_offset                             = 0;
me_skip_mode                                       = 2;
put_start_code                                     = 1;
param_changeable                                   = 0;
changeable_max_bitrate                             = 0;
seq_param_set_id                                   = 0;
profile                                            = 66;
constraint_set_flag                                = 0;
level_type                                         = 1;
level_value                                        = 10;
out_vui_parameters                                 = 1;
chroma_qp_index_offset                             = 0;
constrained_intra_pred                             = 0;
