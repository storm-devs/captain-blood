


enum StringsId
{
	s_form_ok,
	s_form_cancel,
	s_form_close,
	s_form_yes,
	s_form_no,
	s_form_stop,
	s_form_replace,
	s_form_rename,
	s_form_skip,
	s_form_continue,
	s_form_attention_op,
	s_form_warning,
	s_form_cantdoit,
	s_form_error_op,
	s_form_err_rename,
	s_form_err_delete,

	s_quality_best,
	s_quality_medium,
	s_quality_low,

	s_export_err_unknown,
	s_export_err_nowaves,
	s_export_err_nobaseparams,
	s_export_err_noattenuation,
	s_export_err_errorwaveexport,
	s_export_err_wavenotfound,
	s_export_err_reimportwave,
	s_export_err_filenotopen,
	s_export_err_cantwrite,
	s_export_err_nomemory,

	s_export_caption,
	s_export_start_bank,
	s_export_done_bank,
	s_export_done_bank_end,
	s_export_done_bank_errors,

	s_export_msg_of,
	s_export_msg_stage_1,
	s_export_msg_stage_2,
	s_export_msg_1_1,
	s_export_msg_1_2,
	s_export_msg_2_1,
	s_export_msg_2_2,
	s_export_msg_2_3,
	s_export_msg_3_1,
	s_export_msg_3_2,
	s_export_msg_3_3,
	s_export_msg_4_1,
	s_export_msg_5_1,
	s_export_msg_5_2,
	s_export_msg_5_3,
	s_export_msg_6_1,
	s_export_msg_7_1,
	s_export_msg_8_1,
	s_export_msg_8_2,
	s_export_msg_8_3,
	s_export_msg_9_1, 
	s_export_msg_10_1,
	s_export_msg_11_1,
	s_export_msg_12_1,


	s_form_sounds,
	s_form_waves,
	s_form_samples,
	s_form_animation,
	s_form_polygon,
	s_form_game,
	s_form_import,
	s_form_import_error,
	s_form_replace_wave1,
	s_form_replace_wave2,

	s_units_time_sec,

	s_filler,
	
	s_folder_up,
	s_folder_waves,
	s_folder_sounds,
	s_folder_animations,
	s_folder_sounds_global,

	s_waves_sel_wave,
	s_waves_wave_notes,
	s_waves_pc_format,
	s_waves_xbox_format,
	s_waves_sel_exp_format_pc,
	s_waves_sel_exp_format_xbox,
	s_waves_preview_exported,
	s_waves_preview_original,
	s_waves_ignore_pa,
	s_hint_waves_ignore_pa,
	s_waves_ignore_ma,
	s_hint_waves_ignore_ma,
	s_waves_ignore_sln,
	s_hint_waves_ignore_sln,
	s_waves_ignore_clk,
	s_hint_waves_ignore_clk,


	s_snd_priority_min,
	s_snd_priority_normal,
	s_snd_priority_max,
	s_snd_priority_inc,
	s_snd_priority_dec,
	s_snd_count_inf,
	s_snd_vol_db,
	s_snd_banks,
	s_snd_new_bank,
	s_snd_enter_new_bank,
	s_snd_enter_new_sound,
	s_snd_enter_new_sound_folder,
	s_snd_soundbank_options,
	s_snd_soundbank_options_exppath,
	s_snd_sbo_navbutton,
	s_snd_rename_sound,
	s_snd_rename_wave,
	s_snd_rename_folder,
	s_snd_rename_sbank,
	s_snd_delete_items,
	s_snd_delete_sbank,
	s_snd_add_wave_hdr,
	s_snd_graph_minDist,
	s_snd_graph_maxDist,
	s_snd_base_params,
	s_snd_sel_base_params,
	s_snd_rename_base_params,
	s_snd_delete_base_params,
	s_snd_delete_attenuations,
	s_snd_attenuation_curve,
	s_snd_attenuations,
	s_snd_sel_attenuations,
	s_snd_new_attenuations,
	s_snd_enter_new_att,
	s_snd_rename_att,
	s_snd_enter_new_sbp,
	s_snd_rename_sbp,
	s_snd_cant_preview,

	s_snd_select_name,
	s_snd_select_title,
	s_snd_select_hint,
	s_snd_select_rnd,
	s_snd_select_queue,
	s_snd_select_sequence,
	
	s_snd_loop_name,
	s_snd_loop_title,
	s_snd_loop_hint,
	s_snd_loop_disable,
	s_snd_loop_one_wave,
	s_snd_loop_sel_wave_rnd,
	s_snd_loop_sel_wave_que,

	s_snd_fx_name,
	s_snd_fx_title,
	s_snd_fx_hint,
	s_snd_fx_full,
	s_snd_fx_premaster,
	s_snd_fx_master,
	s_snd_fx_music,
	
	s_snd_phoneme_name,
	s_snd_phoneme_title,
	s_snd_phoneme_hint,
	s_snd_phoneme_disable,
	s_snd_phoneme_enable,

	s_snd_sld_volume,
	s_snd_sld_priority,
	s_snd_sld_count,
	s_snd_sld_fx,

	s_snd_waves_silence,
	s_snd_waves_vol,
	s_snd_waves_wgt,
	s_snd_waves_wtime,
	s_snd_waves_atime,
	s_snd_waves_notemploy,

	s_ani_animations,
	s_ani_add_animation,
	s_ani_add_animation_anx,
	s_ani_del_animation,
	s_ani_add_new_movie,
	s_ani_del_movie,
	s_ani_set_mission,
	s_ani_not_in_list,
	s_ani_sound,
	s_ani_locator,
	s_ani_event_local,
	s_ani_event_bind,
	s_ani_event_voice,
	s_ani_event_play,
	s_ani_event_stop,
	s_ani_change_stage,
	s_ani_error_caption,
	s_ani_er_empty_name,
	s_ani_er_name_to_long,
	s_ani_er_already_in_prj,
	s_ani_er_file_not_found,
	s_ani_er_files_to_many,
	s_ani_er_cant_open,
	s_ani_er_evt_exp,
	s_ani_er_noselect,
	s_ani_exp_to,

	s_poly_list_caption,

	s_timeline_del_node,
	s_timeline_del_event,

	s_hint_exit_from_editor,

	s_hint_waves_import,
	s_hint_waves_remove,
	s_hint_waves_rename,
	s_hint_waves_add_folder,
	s_hint_waves_pc_play,
	s_hint_waves_xbox_play,
	s_hint_waves_stop,

	s_hint_sound_ch_sound_bank,

	s_hint_sound_add_new,
	s_hint_sound_add_folder,
	s_hint_sound_rename,
	s_hint_sound_delete,

	s_hint_sound_add_wave,
	s_hint_sound_add_silence,
	s_hint_sound_remove_wave,
	s_hint_sound_attenuations,
	s_hint_sound_att_add,
	s_hint_sound_att_rename,
	s_hint_sound_att_delete,
	s_hint_sound_base_params,
	s_hint_sound_bp_add,
	s_hint_sound_bp_rename,
	s_hint_sound_bp_delete,
	s_hint_sound_play,
	s_hint_sound_stop,

	s_hint_sound_sb_options,
	s_hint_sound_sb_export,
	


	s_hint_ani_add_ani,
	s_hint_ani_add_movie,
	s_hint_ani_set_mis,
	s_hint_ani_rename,	
	s_hint_ani_del_ani,
	s_hint_ani_exps_ani,
	s_hint_ani_expc_ani,
	s_hint_ani_del_movie,
	s_hint_ani_add_node,
	s_hint_ani_sel_sound,
	s_hint_ani_sel_locator,
	s_hint_ani_event_local,
	s_hint_ani_event_bind,
	s_hint_ani_event_voice,
	
	s_hint_ani_tm_play,
	s_hint_ani_tm_play_node,
	s_hint_ani_tm_stop,
	s_hint_ani_tm_prev,
	s_hint_ani_tm_next,
	s_hint_ani_tm_start,
	s_hint_ani_tm_start_node,
	s_hint_ani_tm_end,
	s_hint_ani_tm_end_node,

	s_hint_poly_add_sound,
	s_hint_poly_add_model,
	s_hint_poly_delete,
};

void InitStrings()
{
	strings[s_form_ok] = "Ok";
	strings[s_form_cancel] = "Cancel";
	strings[s_form_close] = "Close";
	strings[s_form_yes] = "Yes";
	strings[s_form_no] = "No";
	strings[s_form_stop] = "Stop";
	strings[s_form_replace] = "Replace";
	strings[s_form_rename] = "Rename";
	strings[s_form_skip] = "Skip";
	strings[s_form_continue] = "Continue";

	strings[s_form_attention_op] = "Attention! Hazard operation!";
	strings[s_form_warning] = "Be careful! No undo for this...";
	strings[s_form_cantdoit] = "Can't do that operation now...";
	strings[s_form_error_op] = "Operation error!";
	strings[s_form_err_rename] = "Error rename this item.";
	strings[s_form_err_delete] = "Error delete some selected items.";

	strings[s_quality_best] = "best quality";
	strings[s_quality_medium] = "medium quality";
	strings[s_quality_low] = "low quality";

	strings[s_export_err_unknown] = "Undescripted error in:";
	strings[s_export_err_nowaves] = "No set waves for sound:";
	strings[s_export_err_nobaseparams] = "No set setup for sound:";
	strings[s_export_err_noattenuation] = "No set attenuation for sound:";
	strings[s_export_err_errorwaveexport] = "Error export wave data:";
	strings[s_export_err_wavenotfound] = "Wave will be delete from project. Sound:";
	strings[s_export_err_reimportwave] = "Wave have oldest internal data. Reimport this wave for fix problem...";
	strings[s_export_err_filenotopen] = "Can't create (open) sound bank file. Open with other program or dont have rights?";
	strings[s_export_err_cantwrite] = "Can't write data to sound bank file. No disk space?";
	strings[s_export_err_nomemory] = "Can't allocate memory for sound bank. Try remove global.ssb and restart editor.";

	strings[s_export_caption] = "Export sound banks...";
	strings[s_export_start_bank] = "Start export sound bank: ";
	strings[s_export_done_bank] = "Sound bank ";
	strings[s_export_done_bank_end] = " export is done...";
	strings[s_export_done_bank_errors] = " have a some problems and can't exported...";

	strings[s_export_msg_of] = " of ";
	strings[s_export_msg_stage_1] = "Stage ";
	strings[s_export_msg_stage_2] = ". ";
	strings[s_export_msg_1_1] = "Prepare sounds export table done...";
	strings[s_export_msg_1_2] = "Prepare sounds export table: ";
	strings[s_export_msg_2_1] = "Start prepare sounds for export...";
	strings[s_export_msg_2_2] = "Prepare sounds for export is done...";
	strings[s_export_msg_2_3] = "Prepare sound for export ";
	strings[s_export_msg_3_1] = "Start prepare waves for export...";
	strings[s_export_msg_3_2] = "Prepare waves for export is done...";
	strings[s_export_msg_3_3] = "Prepare waves for export ";
	strings[s_export_msg_4_1] = "Memory mapping...";
	strings[s_export_msg_5_1] = "Start make sounds table...";
	strings[s_export_msg_5_2] = "Make sounds table is done...";
	strings[s_export_msg_5_3] = "Make sounds table ";
	strings[s_export_msg_6_1] = "Build sound setups table...";
	strings[s_export_msg_7_1] = "Fill sound attenuations table...";
	strings[s_export_msg_8_1] = "Start copy waves data to sound bank...";
	strings[s_export_msg_8_2] = "Copy waves data to sound bank is done...";
	strings[s_export_msg_8_3] = "Copy waves data to sound bank: ";
	strings[s_export_msg_9_1] = "Build fast find table for names...";
	strings[s_export_msg_10_1] = "Fill file headers...";
	strings[s_export_msg_11_1] = "Location binary data...";
	strings[s_export_msg_12_1] = "Save to file...";

	strings[s_form_sounds] = "Sounds";
	strings[s_form_waves] = "Waves";
	strings[s_form_samples] = "Samples";
	strings[s_form_animation] = "Animations";
	strings[s_form_polygon] = "Test polygon";
	strings[s_form_game] = "Game";
	strings[s_form_import] = "Import file:";
	strings[s_form_import_error] = "Error:";
	strings[s_form_replace_wave1] = "Replace wave ";
	strings[s_form_replace_wave2] = " with new file?";
	
	strings[s_units_time_sec] = "s";
	
	strings[s_filler] = "<...>";
	
	strings[s_folder_up] = "..";
	strings[s_folder_waves] = "Waves:";
	strings[s_folder_sounds] = "Sounds:";
	strings[s_folder_animations] = "Animations:";
	strings[s_folder_sounds_global] = "Global";

	strings[s_waves_sel_wave] = "Selected wave: ";
	strings[s_waves_wave_notes] = "Set wave notes: ";
	strings[s_waves_pc_format] = "Wave format for PC:";
	strings[s_waves_xbox_format] = "Wave format for Xbox:";
	strings[s_waves_sel_exp_format_pc] = "Select export format for PC";
	strings[s_waves_sel_exp_format_xbox] = "Select export format for Xbox";
	strings[s_waves_preview_exported] = "preview exported";
	strings[s_waves_preview_original] = "preview original";
	strings[s_waves_ignore_pa] = "ignore pick amplitude";
	strings[s_hint_waves_ignore_pa] = "Ignore pick amplitude defect for current wave.";
	strings[s_waves_ignore_ma] = "ignore midle amplitude";
	strings[s_hint_waves_ignore_ma] = "Ignore midle amplitude defect for current wave.";
	strings[s_waves_ignore_sln] = "ignore silence filling";
	strings[s_hint_waves_ignore_sln] = "Ignore silence filling defect for current wave.";
	strings[s_waves_ignore_clk] = "ignore wave clicks";
	strings[s_hint_waves_ignore_clk] = "Ignore wave clicks defect for current wave.";

	strings[s_snd_priority_min] = "minimum";
	strings[s_snd_priority_normal] = "normal";
	strings[s_snd_priority_max] = "maximum";
	strings[s_snd_priority_inc] = " +";
	strings[s_snd_priority_dec] = " -";
	strings[s_snd_count_inf] = "infinite";
	strings[s_snd_vol_db] = "dB";

	strings[s_snd_banks] = "Sound banks: ";
	strings[s_snd_new_bank] = "Create new sound bank";
	strings[s_snd_enter_new_bank] = "Enter name for new sounds bank:";
	strings[s_snd_enter_new_sound] = "Enter name for new sound:";
	strings[s_snd_enter_new_sound_folder] = "Enter name for new folder:";	
	strings[s_snd_soundbank_options] = "Sound bank options";
	strings[s_snd_soundbank_options_exppath] = "Export sound bank to:";
	strings[s_snd_sbo_navbutton] = "...";
	strings[s_snd_rename_sound] = "Rename sound:";
	strings[s_snd_rename_wave] = "Rename wave:";
	strings[s_snd_rename_folder] = "Rename folder:";
	strings[s_snd_rename_sbank] = "Rename sound bank:";
	strings[s_snd_delete_items] = "Are you sure want delete select items? You have no undo for this...";
	strings[s_snd_delete_sbank] = "Are you sure delete sound bank with all sounds? Have no undo!";
	strings[s_snd_add_wave_hdr] = "Choose waves for add to current sound:";
	strings[s_snd_new_attenuations] = "<Add new attenuation>";

	strings[s_snd_graph_minDist] = "Min. dist.";
	strings[s_snd_graph_maxDist] = "Max. dist.";

	strings[s_snd_base_params] = "Base sound setup";
	strings[s_snd_sel_base_params] = "Select setups";

	strings[s_snd_delete_base_params] = "Are you sure delete this sound setup?  Have no undo!";
	strings[s_snd_delete_attenuations] = "Are you sure delete this attenuation graph?  Have no undo!";


	strings[s_snd_attenuation_curve] = "Attenuation curve for 3D sounds";
	strings[s_snd_attenuations] = "Attenuations";
	strings[s_snd_sel_attenuations] = "Select attenuations";
	strings[s_snd_enter_new_att] = "Enter name for new attenuation graph:";
	strings[s_snd_rename_att] = "Enter new name for attenuation graph:";
	strings[s_snd_enter_new_sbp] = "Enter name for new sound setup:";
	strings[s_snd_rename_sbp] = "Enter new name for sound setup:";
	
	strings[s_snd_cant_preview] = "Can't preview sound, not all parameters is validate now...";

	strings[s_snd_select_name] = "Wave select method:";
	strings[s_snd_select_title] = "Select method";
	strings[s_snd_select_hint] = "Choice select wave method when sound start plaing \n"
								 "[Random] - random select with using wave weight \n"
								 "[Queue] - make virtual waves queue using weights \n"
								 "  When sound start wave get from queue and remove it. \n"
								 "  If queue is empty then generate new one using random method \n"
								 "[Sequence] - select sound waves one by one ";
	strings[s_snd_select_rnd] = "Random";
	strings[s_snd_select_queue] = "Queue";
	strings[s_snd_select_sequence] = "Sequence";

	strings[s_snd_loop_name] = "Sound loop mode:";
	strings[s_snd_loop_title] = "Is loop sound";
	strings[s_snd_loop_hint] = "Enable loop mode for playing wave in current sound ";
	//strings[s_snd_loop_hint] = "Choice loop method for sound\n"
	//						   "[Disable] - no loop sound \n"
	//						   "[One selected wave] - loop wave selected on start play \n"
	//						   "[Random waves] - random select any sound wave for next loop (weight no apply for this) \n"
	//						   "[Queued waves] - random select wave for next loop from waves queue and remove it after select \n"
	//						   "  For queue use all sound waves. When queue empty generate new one by random method ";

	strings[s_snd_loop_disable] = "Disable";
	strings[s_snd_loop_one_wave] = "One selected wave";
	strings[s_snd_loop_sel_wave_rnd] = "Random waves";
	strings[s_snd_loop_sel_wave_que] = "Queued waves";

	strings[s_snd_fx_name] = "Fx connection:";
	strings[s_snd_fx_title] = "Connection points";
	strings[s_snd_fx_hint] = "Choice point for connect sound to mastering effects chain \n"
							 "->Environment \n"
							 "       |      \n"
							 "       V      \n"
							 "-> Premaster  \n"
							 "       |      \n"
							 "       V      \n"
                             "->   Master -> output \n"
							 "       ^      \n"
							 "       |      \n"
							 "->   Music (only for music!)";
	strings[s_snd_fx_full] = "Environment";
	strings[s_snd_fx_premaster] = "Premaster";
	strings[s_snd_fx_master] = "Master";
	strings[s_snd_fx_music] = "Music";

	strings[s_snd_phoneme_name] = "Generate phonemes:";
	strings[s_snd_phoneme_title] = "Generate phonemes";
	strings[s_snd_phoneme_hint] = "Enable or disable generate phonemes data for waves used in sound ";
	strings[s_snd_phoneme_disable] = "Disable";
	strings[s_snd_phoneme_enable] = "Enable";

	strings[s_snd_sld_volume] = "Volume";
	strings[s_snd_sld_priority] = "Priority";
	strings[s_snd_sld_count] = "Count";
	strings[s_snd_sld_fx] = "Connection";

	strings[s_snd_waves_silence] = "silence";
	strings[s_snd_waves_vol] = "vol:";
	strings[s_snd_waves_wgt] = "wgt:";
	strings[s_snd_waves_wtime] = "Wave time: ";
	strings[s_snd_waves_atime] = "Active time: ";
	strings[s_snd_waves_notemploy] = "not employ now...";

	strings[s_ani_animations] = "Animations";
	strings[s_ani_add_animation] = "Add animation to sound project";
	strings[s_ani_add_animation_anx] = "Select anx file for build movies";
	strings[s_ani_del_animation] = "Are you sure delete animation from project?\nAnimation:";
	strings[s_ani_add_new_movie] = "Enter movie name:";
	strings[s_ani_del_movie] = "Are you sure delete movie from project?\nMovie:";
	strings[s_ani_set_mission] = "Set preview mission:";
	strings[s_ani_not_in_list] = "(not in list)";
	strings[s_ani_sound] = "Choice sound";
	strings[s_ani_locator] = "Choice locator";
	strings[s_ani_event_local] = "Connect to locator";
	strings[s_ani_event_bind] = "Bind to node";
	strings[s_ani_event_voice] = "Voice animation";
	strings[s_ani_event_play] = "Play select event";
	strings[s_ani_event_stop] = "Stop playing";
	strings[s_ani_change_stage] = "Are you sure whant change movie stage level?";
	strings[s_ani_error_caption] = "Add new animation error:";
	strings[s_ani_er_empty_name] = "Animation name is not set";
	strings[s_ani_er_name_to_long] = "Animation name is too long";
	strings[s_ani_er_already_in_prj] = "Animation with this name already in project";
	strings[s_ani_er_file_not_found] = "Animation file not found: ";
	strings[s_ani_er_files_to_many] = "Too many animation files with the same name.\nCan be only one uniq animation in project. File: ";
	strings[s_ani_er_cant_open] = "Can't open animation file ";
	strings[s_ani_er_evt_exp] = "Error export animation events!";
	strings[s_ani_er_noselect] = "No select animation";
	strings[s_ani_exp_to] = "Animation events export to file:";

	strings[s_poly_list_caption] = "Scene objects list";
	

	strings[s_timeline_del_node] = "Are you sure delete this node?";
	strings[s_timeline_del_event] = "Are you sure delete this event?";

	strings[s_hint_exit_from_editor] = "Safe exit from editor";

	strings[s_hint_waves_import] = "Import waves to project (F5)";
	strings[s_hint_waves_remove] = "Delete selected waves and folders from project";
	strings[s_hint_waves_rename] = "Rename focused element (wave or folder)";
	strings[s_hint_waves_add_folder] = "Add folder to project's waves tree";
	strings[s_hint_waves_pc_play] = "Play preview for PC export wave";
	strings[s_hint_waves_xbox_play] = "Play preview for PC export wave";
	strings[s_hint_waves_stop] = "Stop play preview wave";	

	strings[s_hint_sound_ch_sound_bank] = "Choice sound bank for work";

	strings[s_hint_sound_add_new] = "Add new sound to current sounds bank";
	strings[s_hint_sound_add_folder] = "Add folder to sounds list";
	strings[s_hint_sound_rename] = "Rename focused element";
	strings[s_hint_sound_delete] = "Delete select elements";

	strings[s_hint_sound_add_wave] = "Add waves to current sound";
	strings[s_hint_sound_add_silence] = "Add silence to current sound";
	strings[s_hint_sound_remove_wave] = "Remove select waves from current sound";
	strings[s_hint_sound_attenuations] = "Choice attenuation form for current sound (3D mode)";
	strings[s_hint_sound_att_add] = "Add new attenuation (copy from current)";
	strings[s_hint_sound_att_rename] = "Rename current attenuation";
	strings[s_hint_sound_att_delete] = "Delete current attenuation";
	strings[s_hint_sound_base_params] = "Choice base parameters setup for current sound";
	strings[s_hint_sound_bp_add] = "Add new sound setup (copy from current)";
	strings[s_hint_sound_bp_rename] = "Rename current sound setup";
	strings[s_hint_sound_bp_delete] = "Delete current sound setup";
	strings[s_hint_sound_play] = "Preview play sound";
	strings[s_hint_sound_stop] = "Stop playing sound";

	strings[s_hint_sound_sb_options] = "Open sound bank options";
	strings[s_hint_sound_sb_export] = "Export selected sound banks";


	strings[s_hint_ani_add_ani] = "Add new animation to sound project";
	strings[s_hint_ani_add_movie] = "Add new movie to animation";
	strings[s_hint_ani_set_mis] = "Set mission for selected movie";
	strings[s_hint_ani_rename] = "Rename movie";
	strings[s_hint_ani_del_ani] = "Delete select animation from sound project";
	strings[s_hint_ani_exps_ani] = "Export events from complete movies for select animation to ini file\nIf need export all events use press <shift> when export\nRemark: Animations with preview events can't load in release game.";
	strings[s_hint_ani_expc_ani] = "Export events from complete movies for current animation to ini file\nIf need export all events use press <shift> when export\nRemark: Animations with preview events can't load in release game.";
	strings[s_hint_ani_del_movie] = "Delete select movies from animation";
	strings[s_hint_ani_add_node] = "Add select node to timeline";
	strings[s_hint_ani_sel_sound] = "Choice the sound for current event";
	strings[s_hint_ani_sel_locator] = "Choice the locator for current event";
	strings[s_hint_ani_event_local] = "Use sound as 3D and connect it to choiced locator";
	strings[s_hint_ani_event_bind] = "When node no play more, sound is stop with fade";
	strings[s_hint_ani_event_voice] = "Get phoneme data form sound and play voice animation";

	strings[s_hint_ani_tm_play] = "Play movie";
	strings[s_hint_ani_tm_play_node] = "Play current node";
	strings[s_hint_ani_tm_stop] = "Stop play";
	strings[s_hint_ani_tm_prev] = "Move play cursor to previos frame";
	strings[s_hint_ani_tm_next] = "Move play cursor to next frame";
	strings[s_hint_ani_tm_start] = "Move play cursor to start movie";
	strings[s_hint_ani_tm_start_node] = "Move play cursor to first frame of current node";
	strings[s_hint_ani_tm_end] = "Move play cursor to end movie";
	strings[s_hint_ani_tm_end_node] = "Move play cursor to end frame of current node";

	strings[s_hint_poly_add_sound] = "Add sound to scene";
	strings[s_hint_poly_add_model] = "Add model to scene";
	strings[s_hint_poly_delete] = "Remove selected sound or model from scene";
}

