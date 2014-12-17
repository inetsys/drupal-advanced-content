<?php

/**
 * Table header definition to manage order in admin/content
 */
function _advanced_content_admin_list_headers(){
	$headers = array(
		'title' => array(
			'data' => t('Title'),
			'type' => 'property',
			'field' => 'title',
			'specifier' => 'title'
		),
		'type' => array(
			'data' => t('Type'),
			'type' => 'property',
			'specifier' => 'type',
			'field' => 'type',
		),
		'author' => t('Author'),
		'status' => t('Status'),
		'changed' => array(
			'data' => t('Updated'),
			'type' => 'property',
			'specifier' => 'changed',
			'field' => 'changed',
			'sort' => 'desc'
		),
	);

	$languages = language_list('enabled');
	$enabled_languages = $languages[1];
	if(count($enabled_languages) > 1){
		$headers['language'] = t('Language');
	}

	$headers['operations'] = array(
		'data' => t('Operations')
	);

	return $headers;
}

/**
 * Query builder to display nodes in admin/content
 */
function _advanced_content_get_content(){
	$header = _advanced_content_admin_list_headers();

	$query = db_select('node', 'n')->extend('PagerDefault')->extend('TableSort');

	$nids = $query
		->fields('n', array('nid'))
		->limit(variable_get('admin_content_elements_per_page', 20))
		->orderByHeader($header)
		->addTag('node_admin_filter')
		->addTag('node_access')
		->execute()
		->fetchCol();

	return $nids ? node_load_multiple($nids) : array();
}

/** Helper function to get node filters from GET and SESSION */
function _advanced_content_get_default_filters(){
	$default_filters = isset($_SESSION['admin_node_filters']) ? $_SESSION['admin_node_filters'] : array();

	foreach($_GET as $key => $value){
		if(strpos($key, 'filter_') === 0){
			$field = substr($key, 7);
			if($field){
				$default_filters[$field] = $value;
				unset($_GET[$key]);
			}
		}
	}
	$_SESSION['admin_node_filters'] = $default_filters;

	return $default_filters;
}
