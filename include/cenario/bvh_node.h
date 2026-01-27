#ifndef BVH_NODE_H
#define BVH_NODE_H

#include "hittable.h"
#include "hittable_list.h"
#include "aabb.h"
#include <algorithm>
#include <memory>
#include <vector>
#include <iostream>

// Comparadores para ordenação de objetos por eixo
inline bool box_x_compare(const std::shared_ptr<hittable>& a, const std::shared_ptr<hittable>& b) {
  aabb box_a, box_b;
  a->bounding_box(box_a);
  b->bounding_box(box_b);
  return box_a.minimum.x() < box_b.minimum.x();
}

inline bool box_y_compare(const std::shared_ptr<hittable>& a, const std::shared_ptr<hittable>& b) {
  aabb box_a, box_b;
  a->bounding_box(box_a);
  b->bounding_box(box_b);
  return box_a.minimum.y() < box_b.minimum.y();
}

inline bool box_z_compare(const std::shared_ptr<hittable>& a, const std::shared_ptr<hittable>& b) {
  aabb box_a, box_b;
  a->bounding_box(box_a);
  b->bounding_box(box_b);
  return box_a.minimum.z() < box_b.minimum.z();
}

class bvh_node : public hittable {
public:
  std::shared_ptr<hittable> left;
  std::shared_ptr<hittable> right;
  aabb box;

  bvh_node() {}

  // Constrói BVH a partir de uma lista de objetos
  bvh_node(std::vector<std::shared_ptr<hittable>>& objects, size_t start, size_t end) {
    // Escolhe eixo aleatório para divisão
    int axis = rand() % 3;
    auto comparator = (axis == 0) ? box_x_compare
                    : (axis == 1) ? box_y_compare
                                  : box_z_compare;

    size_t object_span = end - start;

    if (object_span == 1) {
      left = right = objects[start];
    } else if (object_span == 2) {
      if (comparator(objects[start], objects[start + 1])) {
        left = objects[start];
        right = objects[start + 1];
      } else {
        left = objects[start + 1];
        right = objects[start];
      }
    } else {
      std::sort(objects.begin() + start, objects.begin() + end, comparator);
      
      size_t mid = start + object_span / 2;
      left = std::make_shared<bvh_node>(objects, start, mid);
      right = std::make_shared<bvh_node>(objects, mid, end);
    }

    aabb box_left, box_right;
    
    bool has_left = left->bounding_box(box_left);
    bool has_right = right->bounding_box(box_right);
    
    if (has_left && has_right) {
      box = aabb::surrounding_box(box_left, box_right);
    } else if (has_left) {
      box = box_left;
    } else if (has_right) {
      box = box_right;
    }
  }

  bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
    // Teste rápido com bounding box
    if (!box.hit(r, t_min, t_max))
      return false;

    bool hit_left = left->hit(r, t_min, t_max, rec);
    bool hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);

    return hit_left || hit_right;
  }

  std::string get_name() const override { return "BVH Node"; }

  bool bounding_box(aabb& output_box) const override {
    output_box = box;
    return true;
  }
};

// Classe wrapper que gerencia BVH + objetos sem bounding box (como planos)
class bvh_scene : public hittable {
public:
  std::shared_ptr<bvh_node> bvh_root;
  std::vector<std::shared_ptr<hittable>> unbounded_objects; // Objetos sem AABB (planos)
  
  bvh_scene() {}
  
  void build(std::vector<std::shared_ptr<hittable>>& all_objects) {
    std::vector<std::shared_ptr<hittable>> bounded_objects;
    unbounded_objects.clear();
    
    // Separa objetos com e sem bounding box
    for (auto& obj : all_objects) {
      aabb temp_box;
      if (obj->bounding_box(temp_box)) {
        bounded_objects.push_back(obj);
      } else {
        unbounded_objects.push_back(obj);
      }
    }
    
    if (!bounded_objects.empty()) {
      std::cout << "BVH: " << bounded_objects.size() << " objetos na arvore, "
                << unbounded_objects.size() << " objetos sem bounding box\n";
      bvh_root = std::make_shared<bvh_node>(bounded_objects, 0, bounded_objects.size());
    }
  }
  
  bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
    hit_record temp_rec;
    bool hit_anything = false;
    double closest_so_far = t_max;
    
    // Testa objetos sem bounding box (planos) primeiro
    for (const auto& obj : unbounded_objects) {
      if (obj->hit(r, t_min, closest_so_far, temp_rec)) {
        hit_anything = true;
        closest_so_far = temp_rec.t;
        rec = temp_rec;
      }
    }
    
    // Testa BVH
    if (bvh_root && bvh_root->hit(r, t_min, closest_so_far, temp_rec)) {
      hit_anything = true;
      rec = temp_rec;
    }
    
    return hit_anything;
  }
  
  std::string get_name() const override { return "BVH Scene"; }
  
  bool bounding_box(aabb& output_box) const override {
    if (bvh_root) {
      return bvh_root->bounding_box(output_box);
    }
    return false;
  }
};

#endif
