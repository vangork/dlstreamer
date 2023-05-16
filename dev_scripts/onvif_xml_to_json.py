import json
import xmltodict

def convert_xml_to_json(xml):
    xml_dict = xmltodict.parse(xml)
    json_string = json.dumps(xml_dict, indent=2)
    return json_string

with open('example_xml.xml', 'r') as xml_file:
    xml = xml_file.read()
    json_output = convert_xml_to_json(xml)
    print(json_output)
