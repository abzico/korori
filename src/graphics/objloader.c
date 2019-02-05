#include "graphics/objloader.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "foundation/common.h"

// start with 100 empty space to hold elements
#define INITIAL_ELEM_COUNT 100

// use the same factor as C++ stl uses in vector
#define INCREASE_ELEM_FACTOR 1.5

typedef struct DUP DUP;
struct DUP
{
  VERTEXTEXNORM3D v;
  int idx;
  DUP* next;
};

typedef struct
{
  bool set;
  DUP* dup;
} DUPS;

/// handle vertex for 'f '
static void handle_f_v(int v_index, int vt_index, int vn_index, int* final_vertices_count, int* final_indices_count, VERTEXTEXNORM3D** out_vertices, GLuint** out_indices, int* latest_used_vertices_index, int* indices_index, VERTEXPOS3D* vertices, TEXCOORD2D* texcoords, NORMAL* normals, DUPS* dup_table);

static void free_dup_recur(DUP* dup_ptr)
{
  if (dup_ptr == NULL)
  {
    return;
  }
  else
  {
    free_dup_recur(dup_ptr->next);

    // free this node
    free(dup_ptr);
    dup_ptr->next = NULL;
  }
}

static void free_dups(DUPS** dups_table, int count)
{
  // get value pointed to by double pointer
  DUPS* dups_table_v = *dups_table;

  // free each element of dups table
  for (int i=0; i<count; i++)
  {
    free_dup_recur(dups_table_v[i].dup);
    dups_table_v[i].dup = NULL;
  }

  // free dups table itself
  free(dups_table_v);
  *dups_table = NULL;
}

int KRR_load_objfile(const char* filepath, VERTEXTEXNORM3D** dst_vertices, int* vertices_count, GLuint** dst_indices, int* indices_count)
{
  FILE* file = fopen(filepath, "r");
  if (file == NULL)
  {
    KRR_LOGE("Cannot read .obj file");
    return -1;
  }

  // define line buffer to read from file
  char line[256];
  
  // collect all meta vertex array info before we finally form final vertices and indices
  VERTEXPOS3D* vertices = NULL; 
  TEXCOORD2D* texcoords = NULL;
  NORMAL* normals = NULL;

  // keep track of current allocated count for all buffer
  int v_alloc_count = INITIAL_ELEM_COUNT;
  int vt_alloc_count = INITIAL_ELEM_COUNT;
  int vn_alloc_count = INITIAL_ELEM_COUNT;
  // running indexes for all data
  int vertices_i = 0;
  int texcoords_i = 0;
  int normals_i = 0;

  // start with lowest enough space to hold elements
  // the true number of element is tracked along the way, and
  // we will adjust the number of space later at the end to reduce un-needed memory usage
  vertices = malloc(sizeof(VERTEXPOS3D) * v_alloc_count);
  texcoords = malloc(sizeof(TEXCOORD2D) * vt_alloc_count);
  normals = malloc(sizeof(NORMAL) * vn_alloc_count);

  // empty the memory space
  memset(vertices, 0, sizeof(VERTEXPOS3D) * v_alloc_count);
  memset(texcoords, 0, sizeof(TEXCOORD2D) * vt_alloc_count);
  memset(normals, 0, sizeof(NORMAL) * vn_alloc_count);

  // flag checking whether it has been entered into 'f ' case
  bool entered_f = false;

  // variables related to 'f ' section
  // this will be defered in allocation later after we entered into 'f ' section after we know exact number of elements to allocate
  // from those vertex data
  VERTEXTEXNORM3D* out_vertices = NULL;
  int final_vertices_count = 0;
  int static_vertices_count = 0;

  // same for indices
  GLuint* out_indices = NULL; 
  int final_indices_count = 0;
  
  // latest used vertices index, default to -1
  int latest_used_vertices_index = -1;
  // index for indices, support only for triangle-face for now
  int i_i = 0;

  // duplicate table
  // when such vertex is found, then set this table for such element
  // as well as add duplicated element in the list for future checking
  // in order to give us a correct indice value
  DUPS* v_set_table = NULL;

  // read file line by line
  GLfloat temp_v[3];
  int idx[9];
  while (fgets(line, sizeof(line), file))
  {
    if (strncmp(line, "v ", strlen("v ")) == 0)
    {
      // start at offset 2 as we don't need to pay attention to prefix "v "
      // the same for others
      if (sscanf(line + 2, "%f %f %f", &temp_v[0], &temp_v[1], &temp_v[2]) == 3)
      {
        // check if need to expand the memory space 
        if (vertices_i >= v_alloc_count)
        {
          // expand with pre-set value
          v_alloc_count += v_alloc_count * INCREASE_ELEM_FACTOR;
          vertices = realloc(vertices, sizeof(VERTEXPOS3D) * v_alloc_count);
        }

        // copy by values
        vertices[vertices_i].x = temp_v[0];
        // flip y-axis as exported from modeling tool
        vertices[vertices_i].y = -temp_v[1];
        vertices[vertices_i++].z = temp_v[2];
      }
      else
      {
        KRR_LOGW("Warning: Failed attempt to read vertex data for 'v ' format");
      }
    }
    else if (strncmp(line, "vt ", strlen("vt ")) == 0)
    {
      if (sscanf(line + 3, "%f %f", &temp_v[0], &temp_v[1]) == 2)
      {
        // check if need to expand the memory space 
        if (texcoords_i >= vt_alloc_count)
        {
          // expand with pre-set value
          vt_alloc_count += vt_alloc_count * INCREASE_ELEM_FACTOR;
          texcoords = realloc(texcoords, sizeof(TEXCOORD2D) * vt_alloc_count);
        }

        texcoords[texcoords_i].s = temp_v[0];
        // flip y-axis for texture coordinate as y-axis is flipped for model importing
        texcoords[texcoords_i++].t = 1.0f-temp_v[1];
      }
      else
      {
        KRR_LOGW("Warning: Failed attempt to read vertex data for 'vt ' format");
      }
    }
    else if (strncmp(line, "vn ", strlen("vn ")) == 0)
    {
      if (sscanf(line + 3, "%f %f %f", &temp_v[0], &temp_v[1], &temp_v[2]) == 3)
      {
        // check if need to expand the memory space 
        if (normals_i >= vn_alloc_count)
        {
          // expand with pre-set value
          vn_alloc_count += vn_alloc_count * INCREASE_ELEM_FACTOR;
          normals = realloc(normals, sizeof(NORMAL) * vn_alloc_count);
        }

        normals[normals_i].x = temp_v[0];
        normals[normals_i].y = temp_v[1];
        normals[normals_i++].z = temp_v[2];
      }
      else
      {
        KRR_LOGW("Warning: Failed attempt to read vertex data for 'vn ' format");
      }
    }
    else if (strncmp(line, "f ", strlen("f ")) == 0)
    {
      if (!entered_f)
      {
        // set flag
        entered_f = true;

        // shrink memory space previously allocated for all 3 buffers
        vertices = realloc(vertices, sizeof(VERTEXPOS3D) * vertices_i);
        texcoords = realloc(texcoords, sizeof(TEXCOORD2D) * texcoords_i);
        normals = realloc(normals, sizeof(NORMAL) * normals_i);

        // set number of elements for final vertices as well
        // differentiate from v_count to give us info later how many actual 'v ' is
        final_vertices_count = vertices_i;

        // allocate memory space
        // allocate vertices enough space initially
        out_vertices = malloc(sizeof(VERTEXTEXNORM3D) * final_vertices_count);
        // allocate with initial guess space
        final_indices_count = INITIAL_ELEM_COUNT * 3;
        out_indices = malloc(sizeof(GLuint) * final_indices_count);
        // clear memory space for final vertices, and indices
        memset(out_vertices, 0, sizeof(VERTEXTEXNORM3D) * final_vertices_count);
        memset(out_indices, 0, sizeof(GLuint) * final_indices_count);

        // allocate memory space for dup table
        // number of element for table won't grow as each element might maintain its own duplicate list
        v_set_table = malloc(sizeof(DUPS) * final_vertices_count);
        memset(v_set_table, 0, sizeof(DUPS) * final_vertices_count);

        // save vertices count for later use
        // this value won't be modified by handle_f_v() function
        static_vertices_count = final_vertices_count;
      }

      if (sscanf(line + 2, "%d/%d/%d %d/%d/%d %d/%d/%d", &idx[0], &idx[1], &idx[2], &idx[3], &idx[4], &idx[5], &idx[6], &idx[7], &idx[8]) == 9)
      {
        // vertex 1
        handle_f_v(idx[0]-1, idx[1]-1, idx[2]-1, &final_vertices_count, &final_indices_count, &out_vertices, &out_indices, &latest_used_vertices_index, &i_i, vertices, texcoords, normals, v_set_table);
        // vertex 2
        handle_f_v(idx[3]-1, idx[4]-1, idx[5]-1, &final_vertices_count, &final_indices_count, &out_vertices, &out_indices, &latest_used_vertices_index, &i_i, vertices, texcoords, normals, v_set_table);
        // vertex 3
        handle_f_v(idx[6]-1, idx[7]-1, idx[8]-1, &final_vertices_count, &final_indices_count, &out_vertices, &out_indices, &latest_used_vertices_index, &i_i, vertices, texcoords, normals, v_set_table);
      }
      else
      {
        KRR_LOGW("Warning: Failed attempt to read vertex data for 'f ' format");
      }
    }
  }

  // close file
  fclose(file);
  file = NULL;

  // free un-needed anymore vertex data
  free(vertices);
  vertices = NULL;
  free(texcoords);
  texcoords = NULL;
  free(normals);
  normals = NULL;

  // free dups table
  free_dups(&v_set_table, static_vertices_count);

  // shrink dowm memory of out_vertices, and out_indices
  out_vertices = realloc(out_vertices, sizeof(VERTEXTEXNORM3D) * (latest_used_vertices_index+1));
  out_indices = realloc(out_indices, sizeof(GLuint) * (i_i+1));

  KRR_LOGI("total vertices: %d, texcoords: %d, normals: %d, final vertices: %d, indices count: %d", vertices_i, texcoords_i, normals_i, latest_used_vertices_index+1, i_i+1);
  KRR_LOGI("calculated vertices count: %d, calculated indices count: %d", final_vertices_count, final_indices_count);

  // set results
  if (dst_vertices != NULL)
  {
    *dst_vertices = out_vertices;
  }
  if (vertices_count != NULL)
  {
    *vertices_count = latest_used_vertices_index+1;
  }
  if (dst_indices != NULL)
  {
    *dst_indices = out_indices;
  }
  if (indices_count != NULL)
  {
    *indices_count = i_i+1;
  }

  return 0;
}



void handle_f_v(int v_index, int vt_index, int vn_index, int* final_vertices_count, int* final_indices_count, VERTEXTEXNORM3D** out_vertices, GLuint** out_indices, int* latest_used_vertices_index, int* indices_index, VERTEXPOS3D* vertices, TEXCOORD2D* texcoords, NORMAL* normals, DUPS* dup_table)
{
  // get the pointer to vertices and indices to work with
  VERTEXTEXNORM3D* out_vertices_v = *out_vertices;
  GLuint* out_indices_v = *out_indices;

  if (!dup_table[v_index].set)
  {
    out_vertices_v[v_index].position = vertices[v_index];
    out_vertices_v[v_index].texcoord = texcoords[vt_index];
    out_vertices_v[v_index].normal = normals[vn_index];

    // expand memory space for indices (if need)
    if (*indices_index >= *final_indices_count)
    {
      *final_indices_count = *final_indices_count + *final_indices_count * INCREASE_ELEM_FACTOR;
      *out_indices = realloc(out_indices_v, sizeof(GLuint) * *final_indices_count);
      KRR_LOGI("realloc new size %ld", sizeof(GLuint) * *final_indices_count);
      // update out_indices_v pointer
      out_indices_v = *out_indices;
    }
    // update indices
    out_indices_v[*indices_index] = v_index;
    *indices_index = *indices_index + 1;

    dup_table[v_index].set = true;
  }
  else
  {
    // early check against root if it matches or not
    // so later we can skip checking aginst root
    if (out_vertices_v[v_index].texcoord.s == texcoords[vt_index].s &&
        out_vertices_v[v_index].texcoord.t == texcoords[vt_index].t &&
        out_vertices_v[v_index].normal.x == normals[vn_index].x &&
        out_vertices_v[v_index].normal.y == normals[vn_index].y &&
        out_vertices_v[v_index].normal.z == normals[vn_index].z)
    {
      // expand memory space for indices (if need)
      if (*indices_index >= *final_indices_count)
      {
        *final_indices_count = *final_indices_count + *final_indices_count * INCREASE_ELEM_FACTOR;
        *out_indices = realloc(out_indices_v, sizeof(GLuint) * *final_indices_count);
        // update out_indices_v pointer
        out_indices_v = *out_indices;
      }
      // update indices
      out_indices_v[*indices_index] = v_index;
      *indices_index = *indices_index + 1;
    }
    else
    {
      // if no duplicate, then add itself into the list
      if (dup_table[v_index].dup == NULL)
      {
        // expand memory space for vertices (if need)
        if (*latest_used_vertices_index == -1 ||
            *latest_used_vertices_index >= *final_vertices_count)
        {
          // set proper index
          *latest_used_vertices_index = *final_vertices_count;

          // update vertices count
          *final_vertices_count = *final_vertices_count + *final_vertices_count * INCREASE_ELEM_FACTOR;
          *out_vertices = realloc(out_vertices_v, sizeof(VERTEXTEXNORM3D) * *final_vertices_count);
          // update out_vertices_v pointer
          out_vertices_v = *out_vertices;
        }

        // add into result vertices
        out_vertices_v[*latest_used_vertices_index].position = vertices[v_index];
        out_vertices_v[*latest_used_vertices_index].texcoord = texcoords[vt_index];
        out_vertices_v[*latest_used_vertices_index].normal = normals[vn_index];

        // expand memory space for indices (if need)
        if (*indices_index >= *final_indices_count)
        {
          *final_indices_count = *final_indices_count + *final_indices_count * INCREASE_ELEM_FACTOR;
          *out_indices = realloc(out_indices_v, sizeof(GLuint) * *final_indices_count);
          // update out_indices_v pointer
          out_indices_v = *out_indices;
        }
        // update indices
        out_indices_v[*indices_index] = *latest_used_vertices_index;
        *indices_index = *indices_index + 1;

        // add into duplicate list
        dup_table[v_index].dup = malloc(sizeof(DUP));
        dup_table[v_index].dup->v = out_vertices_v[*latest_used_vertices_index];
        dup_table[v_index].dup->idx = *latest_used_vertices_index;
        dup_table[v_index].dup->next = NULL;

        // update vertices index
        *latest_used_vertices_index = *latest_used_vertices_index + 1;
      }
      // check against duplicate list
      else
      {
        DUP* prev_dup = NULL;
        DUP* dup = dup_table[v_index].dup;
        bool unique = true;

        while ( dup != NULL)
        {
          VERTEXTEXNORM3D v = dup->v;
          // check for duplicate to finish the job earlier
          if (v.texcoord.s == texcoords[vt_index].s &&
              v.texcoord.t == texcoords[vt_index].t &&
              v.normal.x == normals[vn_index].x &&
              v.normal.y == normals[vn_index].y &&
              v.normal.z == normals[vn_index].z)
          {
            unique = false;

            // expand memory space for indices (if need)
            if (*indices_index >= *final_indices_count)
            {
              *final_indices_count = *final_indices_count + *final_indices_count * INCREASE_ELEM_FACTOR;
              *out_indices = realloc(out_indices_v, sizeof(GLuint) * *final_indices_count);
              // update out_indices_v pointer
              out_indices_v = *out_indices;
            }
            // update indices
            out_indices_v[*indices_index] = dup->idx;
            *indices_index = *indices_index + 1;
            break;
          }

          // update previous pointer
          prev_dup = dup;

          // update dup pointer
          dup = dup->next;
        }

        // if still unique, then add into final vertices & duplicate list
        if (unique)
        {
          // expand memory space for vertices (if need)
          if (*latest_used_vertices_index == -1 ||
              *latest_used_vertices_index >= *final_vertices_count)
          {
            // set proper index
            *latest_used_vertices_index = *final_vertices_count;

            // update vertices count
            *final_vertices_count = *final_vertices_count + *final_vertices_count * INCREASE_ELEM_FACTOR;
            *out_vertices = realloc(out_vertices_v, sizeof(VERTEXTEXNORM3D) * *final_vertices_count);
            // update out_vertices_v pointer
            out_vertices_v = *out_vertices;
          }

          // add into result vertices
          out_vertices_v[*latest_used_vertices_index].position = vertices[v_index];
          out_vertices_v[*latest_used_vertices_index].texcoord = texcoords[vt_index];
          out_vertices_v[*latest_used_vertices_index].normal = normals[vn_index];

          // expand memory space for indices (if need)
          if (*indices_index >= *final_indices_count)
          {
            *final_indices_count = *final_indices_count + *final_indices_count * INCREASE_ELEM_FACTOR;
            *out_indices = realloc(out_indices_v, sizeof(GLuint) * *final_indices_count); 
            // update out_indices_v pointer
            out_indices_v = *out_indices;
          }
          // update indices
          out_indices_v[*indices_index] = *latest_used_vertices_index;
          *indices_index = *indices_index + 1;

          // add into duplicate list
          prev_dup->next = malloc(sizeof(DUP));
          prev_dup->next->v = out_vertices_v[*latest_used_vertices_index];
          prev_dup->next->idx = *latest_used_vertices_index;
          prev_dup->next->next = NULL;

          // update vertices index
          *latest_used_vertices_index = *latest_used_vertices_index + 1;
        }
      }
    }
  }
}
