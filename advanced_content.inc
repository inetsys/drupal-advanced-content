<?php

/**
 * Table header definition to manage order in admin/content
 */
function _advanced_content_admin_list_headers(){
	return array(
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
		'status' => array(
			'data' => t('Status'),
			'type' => 'property',
			'specifier' => 'status',
			'field' => 'status',
		),
		'changed' => array(
			'data' => t('Updated'),
			'type' => 'property',
			'specifier' => 'changed',
			'field' => 'changed',
			'sort' => 'desc'
		),
		'language' => array(
			'data' => t('Language'),
			'type' => 'property',
			'specifier' => 'language',
			'field' => 'language',
		),
		'operations' => array(
			'data' => t('Operations'))
	);
}

/**
 * Query builder to display nodes in admin/content
 */
function _advanced_content_get_content(){
	global $pager;

	$header = _advanced_content_admin_list_headers();

	$query = new EntityFieldQuery();
	$query->entityCondition('entity_type', 'node');
	$query->addTag('node_admin_filter');

	if(!user_access('bypass node access')){
		if(user_access('view own unpublished content')){
			$query->condition(db_or()
				->propertyCondition('status', NODE_PUBLISHED)
				->propertyCondition('uid', $GLOBALS['user']->uid)
			);
		}else{
			// If not, restrict the query to published nodes.
			$query->propertyCondition('status', NODE_PUBLISHED);
		}
	}
	$query
		->pager(variable_get('admin_content_elements_per_page', 20))
		->tableSort($header)
		->addTag('node_access');

	$result = $query->execute();

	$pager[$query->pager['element']] = $query->pager;

	return isset($result['node']) ? entity_load('node', array_keys($result['node'])) : array();
}
