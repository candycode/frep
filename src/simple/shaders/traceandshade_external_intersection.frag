//Raytracing of functional representations
//Copyright (c) Ugo Varetto

#version 120

varying vec3 objectSpaceOrigin;
varying vec3 objectSpacePosition;


vec4 ComputeColor( vec3 P, vec3 n );

// intersects and returns number of intersection: {0,1,2}
int Intersect( in vec3 objectSpaceOrigin,
               in vec3 normalizedObjectSpaceDirection,
               out vec3 nearIntersection,
               out vec3 nearIntersectNormal,
               out vec3 farIntersection,
               out vec3 farIntersectionNormal );

void main()
{
  vec3 ipNear;
  vec3 inNear;
  vec3 ipFar;
  vec3 inFar;
  if( Intersect( objectSpaceOrigin, normalize( objectSpacePosition - objectSpaceOrigin ), ipNear, inNear, ipFar, inFar ) == 0 ) discard;
  vec4 P = gl_ModelViewMatrix * vec4( ipNear, 1. );
  P.xyzw /= P.w;
  vec3 N = normalize( gl_NormalMatrix * inNear );
  gl_FragColor = ComputeColor( P.xyz, N );	  
  float z = dot( P, gl_ProjectionMatrixTranspose[ 2 ] );
  float w = dot( P, gl_ProjectionMatrixTranspose[ 3 ] );
  gl_FragDepth = 0.5 * ( z / w + 1.0 );
}


