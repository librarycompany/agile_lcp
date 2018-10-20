<?php

function agile_update_mxml_form($form, $form_state, AbstractObject $object) {
  $mods = $object['MODS']->content;
  var_dump($mods);

}