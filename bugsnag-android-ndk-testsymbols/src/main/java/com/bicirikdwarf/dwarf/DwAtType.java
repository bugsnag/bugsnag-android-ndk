package com.bicirikdwarf.dwarf;

public enum DwAtType {
	DW_AT_sibling(0x0001), //
	DW_AT_location(0x0002), //
	DW_AT_name(0x0003), //
	DW_AT_ordering(0x0009), //
	DW_AT_subscr_data(0x000a), //
	DW_AT_byte_size(0x000b), //
	DW_AT_bit_offset(0x000c), //
	DW_AT_bit_size(0x000d), //
	DW_AT_element_list(0x000f), //
	DW_AT_stmt_list(0x0010), //
	DW_AT_low_pc(0x0011), //
	DW_AT_high_pc(0x0012), //
	DW_AT_language(0x0013), //
	DW_AT_member(0x0014), //
	DW_AT_discr(0x0015), //
	DW_AT_discr_value(0x0016), //
	DW_AT_visibility(0x0017), //
	DW_AT_import(0x0018), //
	DW_AT_string_length(0x0019), //
	DW_AT_common_reference(0x001a), //
	DW_AT_comp_dir(0x001b), //
	DW_AT_const_value(0x001c), //
	DW_AT_containing_type(0x001d), //
	DW_AT_default_value(0x001e), //
	DW_AT_inline(0x0020), //
	DW_AT_is_optional(0x0021), //
	DW_AT_lower_bound(0x0022), //
	DW_AT_producer(0x0025), //
	DW_AT_prototyped(0x0027), //
	DW_AT_return_addr(0x002a), //
	DW_AT_start_scope(0x002c), //
	DW_AT_bit_stride(0x002e), //
	DW_AT_upper_bound(0x002f), //
	DW_AT_abstract_origin(0x0031), //
	DW_AT_accessibility(0x0032), //
	DW_AT_address_class(0x0033), //
	DW_AT_artificial(0x0034), //
	DW_AT_base_types(0x0035), //
	DW_AT_calling_convention(0x0036), //
	DW_AT_count(0x0037), //
	DW_AT_data_member_location(0x0038), //
	DW_AT_decl_column(0x0039), //
	DW_AT_decl_file(0x003a), //
	DW_AT_decl_line(0x003b), //
	DW_AT_declaration(0x003c), //
	DW_AT_discr_list(0x003d), //
	DW_AT_encoding(0x003e), //
	DW_AT_external(0x003f), //
	DW_AT_frame_base(0x0040), //
	DW_AT_friend(0x0041), //
	DW_AT_identifier_case(0x0042), //
	DW_AT_macro_info(0x0043), //
	DW_AT_namelist_item(0x0044), //
	DW_AT_priority(0x0045), //
	DW_AT_segment(0x0046), //
	DW_AT_specification(0x0047), //
	DW_AT_static_link(0x0048), //
	DW_AT_type(0x0049), //
	DW_AT_use_location(0x004a), //
	DW_AT_variable_parameter(0x004b), //
	DW_AT_virtuality(0x004c), //
	DW_AT_vtable_elem_location(0x004d), //
	DW_AT_allocated(0x004e), //
	DW_AT_associated(0x004f), //
	DW_AT_data_location(0x0050), //
	DW_AT_byte_stride(0x0051), //
	DW_AT_entry_pc(0x0052), //
	DW_AT_use_UTF8(0x0053), //
	DW_AT_extension(0x0054), //
	DW_AT_ranges(0x0055), //
	DW_AT_trampoline(0x0056), //
	DW_AT_call_column(0x0057), //
	DW_AT_call_file(0x0058), //
	DW_AT_call_line(0x0059), //
	DW_AT_description(0x005a), //
	DW_AT_binary_scale(0x005b), //
	DW_AT_decimal_scale(0x005c), //
	DW_AT_small(0x005d), //
	DW_AT_decimal_sign(0x005e), //
	DW_AT_digit_count(0x005f), //
	DW_AT_picture_string(0x0060), //
	DW_AT_mutable(0x0061), //
	DW_AT_threads_scaled(0x0062), //
	DW_AT_explicit(0x0063), //
	DW_AT_object_pointer(0x0064), //
	DW_AT_endianity(0x0065), //
	DW_AT_elemental(0x0066), //
	DW_AT_pure(0x0067), //
	DW_AT_recursive(0x0068), //
	DW_AT_signature(0x0069), //
	DW_AT_main_subprogram(0x006a), //
	DW_AT_data_bit_offset(0x006b), //
	DW_AT_const_expr(0x006c), //
	DW_AT_enum_class(0x006d), //
	DW_AT_linkage_name(0x006e), //
	DW_AT_HP_block_index(0x2000), //
	DW_AT_MIPS_fde(0x2001), //
	DW_AT_CPQ_semantic_events(0x2002), //
	DW_AT_MIPS_tail_loop_begin(0x2003), //
	DW_AT_CPQ_split_lifetimes_rtn(0x2004), //
	DW_AT_MIPS_loop_unroll_factor(0x2005), //
	DW_AT_MIPS_software_pipeline_depth(0x2006), //
	DW_AT_MIPS_linkage_name(0x2007), //
	DW_AT_MIPS_stride(0x2008), //
	DW_AT_MIPS_abstract_name(0x2009), //
	DW_AT_MIPS_clone_origin(0x200a), //
	DW_AT_MIPS_has_inlines(0x200b), //
	DW_AT_MIPS_stride_byte(0x200c), //
	DW_AT_MIPS_stride_elem(0x200d), //
	DW_AT_MIPS_ptr_dopetype(0x200e), //
	DW_AT_MIPS_allocatable_dopetype(0x200f), //
	DW_AT_MIPS_assumed_shape_dopetype(0x2010), //
	DW_AT_HP_proc_per_section(0x2011), //
	DW_AT_HP_raw_data_ptr(0x2012), //
	DW_AT_HP_pass_by_reference(0x2013), //
	DW_AT_HP_opt_level(0x2014), //
	DW_AT_HP_prof_version_id(0x2015), //
	DW_AT_HP_opt_flags(0x2016), //
	DW_AT_HP_cold_region_low_pc(0x2017), //
	DW_AT_HP_cold_region_high_pc(0x2018), //
	DW_AT_HP_all_variables_modifiable(0x2019), //
	DW_AT_HP_linkage_name(0x201a), //
	DW_AT_HP_prof_flags(0x201b), //
	DW_AT_INTEL_other_endian(0x2026), //
	DW_AT_sf_names(0x2101), //
	DW_AT_src_info(0x2102), //
	DW_AT_mac_info(0x2103), //
	DW_AT_src_coords(0x2104), //
	DW_AT_body_begin(0x2105), //
	DW_AT_body_end(0x2106), //
	DW_AT_GNU_vector(0x2107), //
	DW_AT_GNU_guarded_by(0x2108), //
	DW_AT_GNU_pt_guarded_by(0x2109), //
	DW_AT_GNU_guarded(0x210a), //
	DW_AT_GNU_pt_guarded(0x210b), //
	DW_AT_GNU_locks_excluded(0x210c), //
	DW_AT_GNU_exclusive_locks_required(0x210d), //
	DW_AT_GNU_shared_locks_required(0x210e), //
	DW_AT_GNU_odr_signature(0x210f), //
	DW_AT_GNU_template_name(0x2110), //
	DW_AT_GNU_call_site_value(0x2111), //
	DW_AT_GNU_call_site_data_value(0x2112), //
	DW_AT_GNU_call_site_target(0x2113), //
	DW_AT_GNU_call_site_target_clobbered(0x2114), //
	DW_AT_GNU_tail_call(0x2115), //
	DW_AT_GNU_all_tail_call_sites(0x2116), //
	DW_AT_GNU_all_call_sites(0x2117), //
	DW_AT_GNU_all_source_call_sites(0x2118), //
	DW_AT_GNU_macros(0x2119), //
	DW_AT_SUN_template(0x2201), //
	DW_AT_SUN_alignment(0x2202), //
	DW_AT_SUN_vtable(0x2203), //
	DW_AT_SUN_count_guarantee(0x2204), //
	DW_AT_SUN_command_line(0x2205), //
	DW_AT_SUN_vbase(0x2206), //
	DW_AT_SUN_compile_options(0x2207), //
	DW_AT_SUN_language(0x2208), //
	DW_AT_SUN_browser_file(0x2209), //
	DW_AT_SUN_vtable_abi(0x2210), //
	DW_AT_SUN_func_offsets(0x2211), //
	DW_AT_SUN_cf_kind(0x2212), //
	DW_AT_SUN_vtable_index(0x2213), //
	DW_AT_SUN_omp_tpriv_addr(0x2214), //
	DW_AT_SUN_omp_child_func(0x2215), //
	DW_AT_SUN_func_offset(0x2216), //
	DW_AT_SUN_memop_type_ref(0x2217), //
	DW_AT_SUN_profile_id(0x2218), //
	DW_AT_SUN_memop_signature(0x2219), //
	DW_AT_SUN_obj_dir(0x2220), //
	DW_AT_SUN_obj_file(0x2221), //
	DW_AT_SUN_original_name(0x2222), //
	DW_AT_SUN_hwcprof_signature(0x2223), //
	DW_AT_SUN_amd64_parmdump(0x2224), //
	DW_AT_SUN_part_link_name(0x2225), //
	DW_AT_SUN_link_name(0x2226), //
	DW_AT_SUN_pass_with_const(0x2227), //
	DW_AT_SUN_return_with_const(0x2228), //
	DW_AT_SUN_import_by_name(0x2229), //
	DW_AT_SUN_f90_pointer(0x222a), //
	DW_AT_SUN_pass_by_ref(0x222b), //
	DW_AT_SUN_f90_allocatable(0x222c), //
	DW_AT_SUN_f90_assumed_shape_array(0x222d), //
	DW_AT_SUN_c_vla(0x222e), //
	DW_AT_SUN_return_value_ptr(0x2230), //
	DW_AT_SUN_dtor_start(0x2231), //
	DW_AT_SUN_dtor_length(0x2232), //
	DW_AT_SUN_dtor_state_initial(0x2233), //
	DW_AT_SUN_dtor_state_final(0x2234), //
	DW_AT_SUN_dtor_state_deltas(0x2235), //
	DW_AT_SUN_import_by_lname(0x2236), //
	DW_AT_SUN_f90_use_only(0x2237), //
	DW_AT_SUN_namelist_spec(0x2238), //
	DW_AT_SUN_is_omp_child_func(0x2239), //
	DW_AT_SUN_fortran_main_alias(0x223a), //
	DW_AT_SUN_fortran_based(0x223b), //
	DW_AT_ALTIUM_loclist(0x2300), //
	DW_AT_use_GNAT_descriptive_type(0x2301), //
	DW_AT_GNAT_descriptive_type(0x2302), //
	DW_AT_upc_threads_scaled(0x3210), //
	DW_AT_PGI_lbase(0x3a00), //
	DW_AT_PGI_soffset(0x3a01), //
	DW_AT_PGI_lstride(0x3a02), //
	DW_AT_APPLE_optimized(0x3fe1), //
	DW_AT_APPLE_flags(0x3fe2), //
	DW_AT_APPLE_isa(0x3fe3), //
	DW_AT_APPLE_block(0x3fe4), //
	DW_AT_APPLE_major_runtime_vers(0x3fe5), //
	DW_AT_APPLE_runtime_class(0x3fe6), //
	DW_AT_APPLE_omit_frame_ptr(0x3fe7), //
	DW_AT_hi_user(0x3fff); //

	private int value;

	DwAtType(int value) {
		this.value = value;
	}

	public static DwAtType byValue(int value) {
		for (DwAtType a : DwAtType.values()) {
			if (a.value == value)
				return a;
		}

		return null;
	}

	public int value() {
		return this.value;
	}
}
