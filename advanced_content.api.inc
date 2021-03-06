<?php

/**
 * Define form components to add to filter form
 * @return array
 */
function hook_node_filter($default_filters = array()){
	return array(
		'body' => array(
			'#type' => 'textfield',
			'#title' => t('Search body'),
			'#default_value' => isset($default_filters['body']) ? $default_filters['body'] : ''
		)
	);
}
