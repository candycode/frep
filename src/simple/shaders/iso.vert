#version 120

//Raytracing of functional representations
//Copyright (c) Ugo Varetto

varying vec3 objectSpaceOrigin;
varying vec3 objectSpacePosition; 
void main()
{

      	  
  bool perspective = abs( gl_ProjectionMatrix[ 3 ][ 3 ] ) < 0.00001 && abs( gl_ProjectionMatrix[ 2 ][ 3 ] ) > 0.000001;
  
  vec4 p = gl_ModelViewMatrix * gl_Vertex;
  //gl_ClipVertex = p;
  gl_Position = gl_ProjectionMatrix * p;
  vec3 rayorigin;      
  if( perspective )
  {
    rayorigin = vec3( 0., 0., 0. );
  }  
  else
  {
    rayorigin = vec3( p.xy/p.w, 0. );
  }
  p = gl_ModelViewMatrixInverse * vec4( rayorigin, 1. );
  objectSpaceOrigin = p.xyz / p.w;
  objectSpacePosition = gl_Vertex.xyz / gl_Vertex.w; 
} 
