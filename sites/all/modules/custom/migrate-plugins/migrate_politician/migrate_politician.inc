<?php

abstract class BasicMigratePolitician extends Migration {

  public function __construct() {
    parent::__construct();
  }

}

class PoliticianMigration extends BasicMigratePolitician {

  public function __construct() {
    parent::__construct();

    // Connect to legacy database and select oldest profile of each politican (oldest=1)
    $query = Database::getConnection('default', 'parlamentwatch')
            ->select('legacy_user', 'u');
    $query->join('legacy_projects', 'p', 'u.cmd = p.cmd');
    $query->fields('u', array('id', 'uuid', 'username', 'title', 'nickname', 'first_name', 'first_name_2nd_lang', 'last_name', 'last_name_2nd_lang', 'first_lang', 'second_lang', 'email', 'gender', 'birthday', 'childs', 'job', 'marriage', 'url_picture', 'party', 'political_roles', 'links'));
    $query->fields('p', array('project', 'startdate'));
    $query->condition('u.oldest', 1, '=');
	switch(variable_get('site_name')){
		case 'abgeordnetenwatch.de':
		    $query->condition('p.project_cmd', array(132, 360));
			break;
		case 'marsad.tn':
		    $query->condition('p.project_cmd', 974);
			break;		
		case 'dailwatch.ie':
		    $query->condition('p.project_cmd', array(233, 486, 30073, 30988)); #, 1036, 1010, 1475
			break;		
		case 'piratenpartei-bayern.de':
		    $query->condition('p.project_cmd', array(1036, 1010, 1475));
			break;				
		default:
		    $query->condition('p.project_cmd', 0);
			break;		
	}


    // Prepare mapping
    $this->source = new MigrateSourceSQL($query);
    $this->destination = new MigrateDestinationUser();
    $this->map = new MigrateSQLMap($this->machineName,
                    array(
                        'id' => array(
                            'type' => 'int',
                            'not null' => TRUE,
							'alias' => 'u',
                        )
                    ),
                    MigrateDestinationUser::getKeySchema()
    );
    
    // Map old to new data fields
    $this->addFieldMapping('name', 'username')->dedupe('users', 'name');
    $this->addFieldMapping('created', 'startdate');
    $this->addFieldMapping('status')->defaultValue(1);
    $this->addFieldMapping('field_user_title', 'title');
    $this->addFieldMapping('uuid', 'uuid');
    $this->addFieldMapping('mail', 'email');
    $this->addFieldMapping('field_user_gender', 'gender');
    $this->addFieldMapping('field_user_birthday', 'birthday');
    $this->addFieldMapping('field_user_pseudonym', 'nickname');
    $this->addFieldMapping('field_user_childs', 'childs');
    $this->addFieldMapping('field_user_job_skills', 'job');
    $this->addFieldMapping('field_user_marriage_status', 'marriage');
    $this->addFieldMapping('role_names')->defaultValue('politician');
    $this->addFieldMapping('field_user_party', 'party')->arguments(array('create_term' => TRUE));
    $this->addFieldMapping('field_user_political_roles', 'political_roles')->separator(';')->arguments(array('create_term' => TRUE));
    $this->addFieldMapping('field_user_picture', 'url_picture');
    $this->addFieldMapping('field_user_more', 'links')->separator('|');
    $this->addFieldMapping('language')->defaultValue('en-gb');
  }

  public function prepareRow($current_row) {
    if($current_row->birthday < 1){
      $current_row->birthday = NULL;
    }
    if($current_row->startdate < 1){
      $current_row->startdate = NULL;
    }
  }
  
  public function complete($current_row){

	$entity = user_load($current_row->uid);
    if($this->sourceValues->second_lang && $this->sourceValues->first_name_2nd_lang){
    
      // This is the array structure for the field you are translating, in my case the name was "name_field" and the language target, English. $value is the translated string.

      $translation = array(
        'translate' => 0,
        'status' => 1,
        'language' => $this->sourceValues->second_lang, // here is the language you're translating to
        'source' => $this->sourceValues->first_lang, // here is the source language
      );

      $handler = entity_translation_get_handler('user', $entity, TRUE); // get the translation handler in place
      // This is the array structure for the field you are translating, in my case the name was "name_field" and the language target, English. $value is the translated string.
      $entity->field_user_first_letter_lname['und'][0]['value'] = substr($this->sourceValues->last_name, '1','2');
      $entity->field_user_fname[$this->sourceValues->first_lang][0]['value'] = $this->sourceValues->first_name;
      $entity->field_user_lname[$this->sourceValues->first_lang][0]['value'] = $this->sourceValues->last_name;
      $entity->field_user_fname[$this->sourceValues->second_lang][0]['value'] = $this->sourceValues->first_name_2nd_lang;
      $entity->field_user_lname[$this->sourceValues->second_lang][0]['value'] = $this->sourceValues->last_name_2nd_lang;
      $values = array(
        'user_fname' => array (
          $this->sourceValues->second_lang => array(
            '0' => array(
              'value' => $this->sourceValues->first_name_2nd_lang,
             ),
          ),
          $this->sourceValues->first_lang => array(
            '0' => array(
              'value' => $this->sourceValues->first_name,
             )
           )
         ),
        'user_lname' => array (
          $this->sourceValues->second_lang => array(
            '0' => array(
              'value' => $this->sourceValues->last_name_2nd_lang,
             )
           ),
          $this->sourceValues->first_lang => array(
            '0' => array(
              'value' => $this->sourceValues->last_name,
             )
           )
         ),
          
      );
      /*$values = array(
        'field_user_fname' => array (
          $this->sourceValues->first_lang => array(
            '0' => array(
              'value' => $this->sourceValues->first_name,
             )
           )
         ),
        'field_user_lname' => array (
          $this->sourceValues->first_lang => array(
            '0' => array(
              'value' => $this->sourceValues->last_name,
             )
           )
         ),
      );*/
      // Finally you set the translation and save the object.
      $handler->setTranslation($translation, $values);
      //print_r($entity);
      
    }
	else{
$lang_obj = variable_get('language_default');
     $entity->field_user_fname['und'][0]['value'] = $this->sourceValues->first_name;
     $entity->field_user_lname['und'][0]['value'] = $this->sourceValues->last_name;
     $entity->field_user_first_letter_lname['und'][0]['value'] = substr($this->sourceValues->last_name, 0,1);
     //$entity->field_user_fname['und'][0]['value'] = $this->sourceValues->first_name;
     $entity->field_user_fname[$lang_obj->language][0]['value'] = $this->sourceValues->first_name;
     $entity->field_user_lname[$lang_obj->language][0]['value'] = $this->sourceValues->last_name;
     $entity->revision = 0;
	}
	$entity->revision = 0;
	user_save($entity);
  }

}
