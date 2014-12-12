<?php

/** Menu callback of admin/content */
function _advanced_content_admin_page($form, $form_state){
	module_load_include('inc', 'advanced_content');

	$form['#attached']['css'][] = drupal_get_path('module', 'advanced_content').'/advanced_content.admin.css';

	$languages = language_list();
	$destination = drupal_get_destination();
	$nodes = _advanced_content_get_content();

	$header = _advanced_content_admin_list_headers();
	$body = array();

	foreach($nodes as $node){
		$langcode = entity_language('node', $node);
		$l_options = $langcode != LANGUAGE_NONE && isset($languages[$langcode]) ? array('language' => $languages[$langcode]) : array();
		$body[$node->nid] = array(
			'title' => array(
				'data' => array(
					'#type' => 'link',
					'#title' => $node->title,
					'#href' => 'node/' . $node->nid,
					'#options' => $l_options,
					'#suffix' => ' ' . theme('mark', array('type' => node_mark($node->nid, $node->changed))),
				),
			),
			'type' => check_plain(node_type_get_name($node)),
			'author' => theme('username', array('account' => $node)),
			'status' => $node->status ? t('published') : t('not published'),
			'changed' => format_date($node->changed, 'short'),
		);

		// Multilanguage
		if($langcode == LANGUAGE_NONE || isset($languages[$langcode])){
			$body[$node->nid]['language'] = $langcode == LANGUAGE_NONE ? t('Language neutral') : t($languages[$langcode]->name);
		}else{
			$body[$node->nid]['language'] = t('Undefined language (@langcode)', array('@langcode' => $langcode));
		}

		$operations = array();
		if (node_access('update', $node)) {
			$operations['edit'] = array(
				'title' => t('edit'),
				'href' => 'node/' . $node->nid . '/edit',
				'query' => $destination,
			);
		}
		if (node_access('delete', $node)) {
			$operations['delete'] = array(
				'title' => t('delete'),
				'href' => 'node/' . $node->nid . '/delete',
				'query' => $destination,
			);
		}
		$body[$node->nid]['operations'] = array();
		if (count($operations) > 1) {
			// Render an unordered list of operations links.
			$body[$node->nid]['operations'] = array(
				'data' => array(
					'#theme' => module_exists('ctools') ? 'links__ctools_dropbutton' : 'links__node_operations',
					'#links' => $operations
				),
			);
		}
		elseif (!empty($operations)) {
			// Render the first and only operation as a link.
			$link = reset($operations);
			$body[$node->nid]['operations'] = array(
				'data' => array(
					'#type' => 'link',
					'#title' => $link['title'],
					'#href' => $link['href'],
					'#options' => array('query' => $link['query']),
				),
			);
		}
	}

	// Filters
	$default_filters = isset($_SESSION['admin_node_filters']) ? $_SESSION['admin_node_filters'] : array();
	$form['filter'] = array(
		'#type' => 'fieldset',
		'#title' => t('Filters'),
		'#collapsible' => true,
		'#collapsed' => empty($default_filters),
		'#tree' => true,
		'#weight' => 2,
	);
	foreach(module_invoke_all('node_filter') as $filter => $data) {
		$form['filter'][$filter] = $data;
	}
	$form['filter']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Apply'),
		'#submit' => array('_advanced_content_admin_page_filter_submit'),
	);
	$form['filter']['reset'] = array(
		'#type' => 'submit',
		'#value' => t('Reset'),
		'#submit' => array('_advanced_content_admin_page_filter_reset'),
	);

	if(user_access('administer nodes')){
		// Advanced actions
		$form['options'] = array(
			'#type' => 'fieldset',
			'#title' => t('Advanced actions'),
			'#collapsible' => true,
			'#collapsed' => true,
			'#attributes' => array('class' => array('container-inline')),
			'#weight' => 1
		);
		$options = array();

		// Implementation of hook_node_operations in module node is located in node.admin.inc
		module_load_include('inc', 'node', 'node.admin');

		foreach(module_invoke_all('node_operations') as $operation => $array) {
			$options[$operation] = $array['label'];
		}
		$form['options']['operation'] = array(
			'#type' => 'select',
			'#title' => t('Operation'),
			'#title_display' => 'invisible',
			'#options' => $options,
			'#default_value' => 'approve',
		);
		$form['options']['submit'] = array(
			'#type' => 'submit',
			'#value' => t('Update'),
			'#validate' => array('node_admin_nodes_validate'),
			'#submit' => array('node_admin_nodes_submit'),
		);

		// Node list
		$form['nodes'] = array(
			'#type' => 'tableselect',
			'#header' => $header,
			'#options' => $body,
			'#empty' => t('No content available.'),
			'#weight' => 3
		);
	}else {
		$form['nodes'] = array(
			'#theme' => 'table',
			'#header' => $header,
			'#rows' => $body,
			'#empty' => t('No content available.'),
			'#weight' => 3
		);
	}

	// Pager info
	global $pager_page_array, $pager_total, $pager_total_items, $pager_limits;
	$element = 0;
	$total = $pager_total_items[$element];
	$num_page = $pager_page_array[$element];
	$elements_per_page = $pager_limits[$element];
	$first = $num_page * $elements_per_page + 1;
	$last = min($total, ($num_page + 1) * $elements_per_page);

	if($total){
		if($first == $last){
			$pager_message = t('Showing element !index of a total of !total', array('!index' => $first, '!total' => $total));
		}else{
			$pager_message = t('Showing elements !first to !last of a total of !total', array('!first' => $first, '!last' => $last, '!total' => $total));
		}
		$form['pager_info'] = array(
			'#markup' => $pager_message,
			'#weight' => 4
		);
		$form['pager'] = array(
			'#markup' => theme('pager'),
			'#weight' => 5
		);
	}

	return $form;
}

/**
 * Submit callback of filter form in admin/content
 * Submit button
 */
function _advanced_content_admin_page_filter_submit($form, &$form_state){
	if(isset($_GET['page'])){
		$_GET['page'] = 0;
	}
	if(isset($_GET['destination'])){
		unset($_GET['destination']);
	}

	$filters = $form_state['values']['filter'];
	unset($filters['submit']);
	unset($filters['reset']);

	foreach($filters as $field => $value){
		if(empty($value) && $value !== false && $value !== 0 && $value !== '0'){
			unset($filters[$field]);
		}
	}

	$_SESSION['admin_node_filters'] = $filters;
}

/**
 * Submit callback of filter form in admin/content
 * Reset button
 */
function _advanced_content_admin_page_filter_reset($form, &$form_state){
	if(isset($_GET['page'])){
		$_GET['page'] = 0;
	}
	if(isset($_GET['destination'])){
		unset($_GET['destination']);
	}

	$_SESSION['admin_node_filters'] = array();
}

/** Menu callback of admin/structure/menu/manage/add_existing_content */
function _advanced_content_select_content($form, $form_state){
	module_load_include('inc', 'advanced_content');

	$form['#attached']['css'][] = drupal_get_path('module', 'advanced_content').'/advanced_content.admin.css';

	$languages = language_list();
	$nodes = _advanced_content_get_content();

	$header = _advanced_content_admin_list_headers();
	$body = array();

	// Query string for 'select' links
    module_load_include('inc', 'content_menu', 'content_menu.menu_admin');
    $menu_item = content_menu_get_menu_item_from_querystring();
    $action_url = 'admin/structure/menu/manage/' . $menu_item['name'] . '/add';

	foreach($nodes as $node){
		$langcode = entity_language('node', $node);
		$l_options = $langcode != LANGUAGE_NONE && isset($languages[$langcode]) ? array('language' => $languages[$langcode]) : array();
		$l_options['attributes']['target'] = '_blank';
		$l_options['attributes']['title'] = t('Open in new window');
		$body[$node->nid] = array(
			'title' => array(
				'data' => array(
					'#type' => 'link',
					'#title' => $node->title,
					'#href' => 'node/' . $node->nid,
					'#options' => $l_options,
				),
			),
			'type' => check_plain(node_type_get_name($node)),
			'author' => theme('username', array('account' => $node)),
			'status' => $node->status ? t('published') : t('not published'),
			'changed' => format_date($node->changed, 'short'),
		);

		if ($langcode == LANGUAGE_NONE || isset($languages[$langcode])) {
			$body[$node->nid]['language'] = $langcode == LANGUAGE_NONE ? t('Language neutral') : t($languages[$langcode]->name);
		}
		else {
			$body[$node->nid]['language'] = t('Undefined language (@langcode)', array('@langcode' => $langcode));
		}

		$operations = array();

		$params = $menu_item;
		$params['link_path'] = 'node/'.$node->nid;
		$params['destination'] = 'admin/structure/menu/manage/'.$params['name'];
		$query = content_menu_assemble_query_string($params);
		$operations['edit'] = array(
			'title' => t('Select'),
			'href' => $action_url,
			'query' => $query,
		);

		$body[$node->nid]['operations'] = array(
			'data' => array(
				'#theme' => module_exists('ctools') ? 'links__ctools_dropbutton' : 'links__node_operations',
				'#links' => $operations,
				'#class' => array('ctools-button-processed')
			),
		);
	}

	// Filters
	$default_filters = isset($_SESSION['admin_node_filters']) ? $_SESSION['admin_node_filters'] : array();
	$form['filter'] = array(
		'#type' => 'fieldset',
		'#title' => t('Filters'),
		'#collapsible' => true,
		'#collapsed' => empty($default_filters),
		'#tree' => true,
		'#weight' => 2,
	);
	foreach(module_invoke_all('node_filter') as $filter => $data) {
		$form['filter'][$filter] = $data;
	}
	$form['filter']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Apply'),
		'#submit' => array('_advanced_content_admin_page_filter_submit'),
	);
	$form['filter']['reset'] = array(
		'#type' => 'submit',
		'#value' => t('Reset'),
		'#submit' => array('_advanced_content_admin_page_filter_reset'),
	);

	// List
	$form['nodes'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $body,
		'#empty' => t('No content available.'),
		'#weight' => 3
	);

	// Pager
	global $pager;
	$pager_info = $pager[0];
	$first = $pager_info['start'] + 1;
	$last = min($pager_info['total'], $pager_info['start'] + $pager_info['limit']);
	$total = $pager_info['total'];
	// Pager info
	if($total){
		if($first == $last){
			$pager_message = t('Showing element !index of a total of !total', array('!index' => $first, '!total' => $total));
		}else{
			$pager_message = t('Showing elements !first to !last of a total of !total', array('!first' => $first, '!last' => $last, '!total' => $total));
		}
		$form['pager_info'] = array(
			'#markup' => $pager_message,
			'#weight' => 4
		);
		$form['pager'] = array(
			'#markup' => theme('pager'),
			'#weight' => 5
		);
	}

	return $form;
}
