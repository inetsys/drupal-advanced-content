<?php

/**
 * Define form components to add to filter form
 * @return array
 */
function hook_node_filter(){
	$default_filters = isset($_SESSION['admin_node_filters']) ? $_SESSION['admin_node_filters'] : array();

	return array(
		'body' => array(
			'#type' => 'textfield',
			'#title' => t('Search body'),
			'#default_value' => isset($default_filters['body']) ? $default_filters['body'] : ''
		)
	);
}

/**
 * Implementation of hook_entity_query_alter
 * Must add field/property conditions to query to apply filters
 * @param EntityFieldQuery $query
 */
function hook_entity_query_alter($query){
	if(in_array('node_admin_filter', $query->tags) && isset($_SESSION['admin_node_filters']) && isset($_SESSION['admin_node_filters']['body'])){
		$query->fieldCondition('body', 'value', $_SESSION['admin_node_filters']['body'], 'CONTAINS');
	}
}