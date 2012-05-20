//Raytracing of functional representations
//Copyright (c) Ugo Varetto

#version 120
//vec4 color;
vec3 rayorigin;
vec3 raydir;
vec4 origin;
float eps = 0.000001;
uniform vec3 halfBoxSize;
uniform mat4 primitiveModelviewInv;

uniform float deltaStep;

uniform vec4 viewport; //!!!! x,y, width,height

// declaration of isofun and gradient
float IsoFunction( in vec3 p );
vec3 IsoGradient( in vec3 p );


void ComputeRay( float offsetX, float offsetY )
{
  vec3 ssP = gl_FragCoord.xyz;
  ssP.xy += vec2( offsetX, offsetY );
  ssP.xy = 2.0 * ( ( ssP.xy - viewport.xy ) / viewport.zw ) - vec2( 1.0 );
  ssP.z *= 2.0;
  ssP.z -= 1.;
  vec4 P = gl_ProjectionMatrixInverse * vec4( ssP, 1.0 );/*ModelView*/
  P /= P.w;
  raydir = ( /*primitiveModelviewInv **/ P ).xyz;
  rayorigin = vec3( 0., 0., 0. );
  origin = gl_ModelViewMatrixInverse /** primitiveModelviewInv*/ * vec4( rayorigin, 1. );
  origin.xyz /= origin.w;  
}


#define COMPUTE_COLOR
#ifndef COMPUTE_COLOR
vec3 lightDir = normalize( vec3( 0., -1., -1. ) );
float kd = 1.0;
float ka = 0.01;
float ks = 1.;
float sh = 100.;
vec3 refcolor = vec3( 1, 1, 1 );
vec4 ComputeColor( vec3 P, vec3 n )
{
  vec3 N = faceforward( n, lightDir, n );
  float d = dot( N, -lightDir );
  float s = pow( max( 0.0, dot( vec3( 0, 0, 1 ), reflect( lightDir, N ) ) ), sh );
  float a = color.a * ( 1. - dot( vec3( 0., 0., 1 ), n ) );
  return vec4(  ks * s * refcolor + kd * d * color.rgb + ka * color.rgb, a );
}
#else
vec4 ComputeColor( vec3 P, vec3 n );
#endif

//------------------------------------------------------------------------------
struct ColorDepth
{
  vec4 color;
  float depth;
};

float IsoFunctionAverage( vec3 P )
{
  return IsoFunction( P );
  const float w = 1./6.;  
  float f1 = IsoFunction( P + vec3( deltaStep, 0., 0. ) );
  float f2 = IsoFunction( P + vec3( -deltaStep, 0., 0. ) );  
  float f3 = IsoFunction( P + vec3( 0., deltaStep, 0. ) );
  float f4 = IsoFunction( P + vec3( 0., -deltaStep, 0. ) );
  float f5 = IsoFunction( P + vec3( 0., 0., deltaStep ) );
  float f6 = IsoFunction( P + vec3( 0., 0., -deltaStep ) );
  //float f7 = IsoFunction( P );    	
  return w * ( f1 + f2 + f3 + f4 + f5 + f6 );// + f7 );
}


ColorDepth RayTrace( float offsetX, float offsetY )
{

  ComputeRay( offsetX, offsetY );

  float tstart = length( raydir ) - 0.001;
  float tend = tstart + 6.0;
  float tstep = deltaStep;
  float dTstep = 0.01 * tstep;
    
  vec3 rdir = transpose( gl_NormalMatrix ) * raydir;
    
  vec3 dir = normalize( rdir );
  int numSteps = int( ( tend - tstart ) / tstep );
  vec3 prev = origin.xyz + tstart * dir;
  float tprev = tstart;
  vec3 cur = prev;
  float tcur = tprev;
  float t = -1.0;
  for( int i = 1; i < numSteps; ++i )
  {
    tcur = tstart + tstep * float( i );
    cur = origin.xyz + tcur * dir;
    float f1 = IsoFunctionAverage( prev );
    float f2 = IsoFunctionAverage( cur ); 
    if( f1 * f2 < 0. )
    {
      cur = prev + abs(f1 /(f1 - f2)) * ( cur - prev );
      t = tcur;
      break; 
    }
    else if( abs( f2 ) < eps )
    {
         
         t = length( cur - prev );
         break;
    }
    else if( abs( f1 ) < eps )
    {
         cur = prev;
         t = 0.;
         break;
    }
    else prev = cur;
    /*vec4 P = gl_ModelViewMatrix * vec4( cur, 1. );
    P.xyzw /= P.w;
    float z = dot( P, gl_ProjectionMatrixTranspose[ 2 ] );
    float w = dot( P, gl_ProjectionMatrixTranspose[ 3 ] );
    gl_FragDepth = 0.5 * ( z / w + 1.0 );
    tstep = deltaStep / ( 1. - gl_FragDepth );*/
    tstep += dTstep;
  }
  if( t < 0.0 ) discard;
 
  vec4 P = gl_ModelViewMatrix * vec4( cur, 1. );
  P.xyzw /= P.w;
  vec3 N = normalize( gl_NormalMatrix * IsoGradient( cur ) );
  gl_FragColor = ComputeColor( P.xyz, N );	  
  float z = dot( P, gl_ProjectionMatrixTranspose[ 2 ] );
  float w = dot( P, gl_ProjectionMatrixTranspose[ 3 ] );
  gl_FragDepth = 0.5 * ( z / w + 1.0 );
  ColorDepth cd;
  cd.color = gl_FragColor;
  cd.depth = gl_FragDepth;
  return cd;
}


void main(void)
{
  /*ColorDepth cd  = RayTrace( 0., 0. );
  ColorDepth cd1 = RayTrace( -.5, 0.0 );
  ColorDepth cd2 = RayTrace( -.5, .5 );
  ColorDepth cd3 = RayTrace( 0.0, 0.5 );
  ColorDepth cd4 = RayTrace( 0.5, 0.5 );
  ColorDepth cd5 = RayTrace( 0.5, 0.0 );
  ColorDepth cd6 = RayTrace( 0.5, -.5 );
  ColorDepth cd7 = RayTrace( 0.0, -.5 );
  ColorDepth cd8 = RayTrace( -.5, -.5 );
  const float c = 1./9.;
  gl_FragColor = c * cd.color + ( 1. - c ) * .125 *
   ( cd1.color + cd2.color + cd3.color + cd4.color + 
     cd5.color + cd6.color + cd7.color + cd8.color );
  gl_FragDepth = c * cd.depth + ( 1. - c ) * .125 * 
   ( cd1.depth + cd2.depth + cd3.depth + cd4.depth +
     cd5.depth + cd6.depth + cd7.depth + cd8.depth );*/
  gl_FragColor = RayTrace( 0., 0. ).color;
  gl_FragDepth = RayTrace( 0., 0. ).depth;   
}


